/*
 * ymodem.c
 *
 *  Created on: Jul 16, 2025
 *      Author: 2005b
 */

#include "ymodem.h"
#include "uart.h"
#include "cli.h"

#include "ff.h"
#include "fatfs.h"
#include <string.h>

#define YMODEM_SOH    0x01
#define YMODEM_STX    0x02
#define YMODEM_ACK    0x06
#define YMODEM_NACK   0x15
#define YMODEM_EOT    0x04
#define YMODEM_C      0x43
#define YMODEM_CAN    0x18
#define YMODEM_BS     0x08



enum
{
  YMODEM_STATE_WAIT_HEAD,
  YMODEM_STATE_WAIT_FIRST,
  YMODEM_STATE_WAIT_DATA,
  YMODEM_STATE_WAIT_LAST,
  YMODEM_STATE_WAIT_END,
  YMODEM_STATE_WAIT_CANCEL,
};

enum
{
  YMODEM_PACKET_WAIT_FIRST,
  YMODEM_PACKET_WAIT_SEQ1,
  YMODEM_PACKET_WAIT_SEQ2,
  YMODEM_PACKET_WAIT_DATA,
  YMODEM_PACKET_WAIT_CRCH,
  YMODEM_PACKET_WAIT_CRCL,
};



#ifdef _USE_HW_CLI
static void cliYmodem(cli_args_t *args);
#endif

static FIL   yfile;
static UINT  ybytes_written;
extern FATFS SDFatFs;
extern char SDPath[4];
static uint16_t crc16(uint8_t *data, uint16_t size);
static bool ymodemReceivePacket(ymodem_packet_t *p_packet, uint8_t data_in);

// 1) 스트리밍 버퍼링 변수
static uint8_t  stream_buf[STREAM_BUF_SIZE];
static size_t   stream_len = 0;

// 2) 버퍼에 모아서 한 번에 쓰는 함수
static void Stream_Write(uint8_t *data, uint16_t len)
{
  size_t space = STREAM_BUF_SIZE - stream_len;
  while (len > 0) {
    size_t w = (len < space) ? len : space;
    memcpy(&stream_buf[stream_len], data, w);
    stream_len += w;
    data       += w;
    len        -= w;
    space       = STREAM_BUF_SIZE - stream_len;

    // 버퍼가 가득 찼으면 실제로 SD에 기록
    if (stream_len == STREAM_BUF_SIZE) {
      f_write(&yfile, stream_buf, stream_len, &ybytes_written);
      stream_len = 0;
      space      = STREAM_BUF_SIZE;
    }
  }
}

// 3) 전송 종료 후 남은 데이터 기록 및 닫기
static void Stream_Flush(void)
{
  if (stream_len > 0) {
    f_write(&yfile, stream_buf, stream_len, &ybytes_written);
    stream_len = 0;
  }
  f_close(&yfile);
}


bool ymodemInit(void)
{
#ifdef _USE_HW_CLI
  cliAdd("ymodem", cliYmodem);
#endif

  return true;
}

bool ymodemOpen(ymodem_t *p_modem, uint8_t ch)
{
  bool ret = true;

  p_modem->ch = ch;
  p_modem->is_init = true;

  p_modem->state           = YMODEM_STATE_WAIT_HEAD;
  p_modem->rx_packet.state = YMODEM_PACKET_WAIT_FIRST;
  p_modem->file_buf        = &p_modem->rx_packet.buffer[3];
  p_modem->file_buf_length = 0;
  p_modem->pre_time        = millis();
  p_modem->start_time      = 3000;

  p_modem->rx_packet.data = &p_modem->rx_packet.buffer[3];

  return ret;
}

bool ymodemPutch(ymodem_t *p_modem, uint8_t data)
{
  bool ret = true;

  ret = uartWrite(p_modem->ch, &data, 1);

  return ret;
}

bool ymodemGetFileInfo(ymodem_t *p_modem)
{
  bool ret = true;
  bool valid;
  uint16_t size_i;

  valid = false;
  for (int i=0; i<128; i++)
  {
    p_modem->file_name[i] = p_modem->rx_packet.data[i];
    if (p_modem->file_name[i] == 0x00)
    {
      size_i = i + 1;
      valid = true;
      break;
    }
  }

  if (valid == true)
  {
    for (int i=size_i; i<128; i++)
    {
      if (p_modem->rx_packet.data[i] == 0x20)
      {
        p_modem->rx_packet.data[i] = 0x00;
        break;
      }
    }

    p_modem->file_length = (uint32_t)strtoul((const char * )&p_modem->rx_packet.data[size_i], (char **)NULL, (int) 0);
  }

  return ret;
}

bool ymodemReceive(ymodem_t *p_modem)
{
  bool ret = false;
  bool update = false;
  uint32_t buf_length;


  if (p_modem->is_init != true)
  {
    p_modem->type = YMODEM_TYPE_ERROR;
    return true;
  }

  if (uartAvailable(p_modem->ch) > 0)
  {
    p_modem->rx_data = uartRead(p_modem->ch);
    update = true;

    //uartPrintf(_DEF_UART1, "Rx 0x%X, %d\n", p_modem->rx_data, p_modem->rx_packet.state);
  }

  if (update == true && ymodemReceivePacket(&p_modem->rx_packet, p_modem->rx_data) == true)
  {
    //uartPrintf(_DEF_UART1, "RxPacket 0x%X\n", p_modem->rx_packet.stx);

    if (p_modem->state != YMODEM_STATE_WAIT_HEAD)
    {
      if (p_modem->rx_packet.stx == YMODEM_CAN)
      {
        p_modem->state = YMODEM_STATE_WAIT_CANCEL;
      }
    }

    switch(p_modem->state)
    {
      case YMODEM_STATE_WAIT_HEAD:
        if (p_modem->rx_packet.stx == YMODEM_EOT)
        {
          ymodemPutch(p_modem, YMODEM_NACK);
          p_modem->state = YMODEM_STATE_WAIT_LAST;
        }
        else if (p_modem->rx_packet.seq[0] == 0x00)
        {
          p_modem->file_addr = 0;
          ymodemGetFileInfo(p_modem);

          ymodemPutch(p_modem, YMODEM_ACK);
          ymodemPutch(p_modem, YMODEM_C);

          p_modem->state = YMODEM_STATE_WAIT_FIRST;
          p_modem->type = YMODEM_TYPE_START;
          ret = true;
        }
        break;

      case YMODEM_STATE_WAIT_FIRST:
        if (p_modem->rx_packet.stx == YMODEM_EOT)
        {
          ymodemPutch(p_modem, YMODEM_NACK);
          p_modem->state = YMODEM_STATE_WAIT_LAST;
        }
        else if (p_modem->rx_packet.seq[0] == 0x01)
        {
          p_modem->file_addr = 0;
          p_modem->file_received = 0;

          buf_length = (p_modem->file_length - p_modem->file_addr);
          if (buf_length > p_modem->rx_packet.length)
          {
            buf_length = p_modem->rx_packet.length;
          }
          p_modem->file_buf_length = buf_length;
          p_modem->file_received += buf_length;

          ymodemPutch(p_modem, YMODEM_ACK);

          p_modem->state = YMODEM_STATE_WAIT_DATA;
          p_modem->type = YMODEM_TYPE_DATA;
          ret = true;
        }
        break;

      case YMODEM_STATE_WAIT_DATA:
        if (p_modem->rx_packet.stx == YMODEM_EOT)
        {
          ymodemPutch(p_modem, YMODEM_NACK);
          p_modem->state = YMODEM_STATE_WAIT_LAST;
        }
        else
        {
          buf_length = (p_modem->file_length - p_modem->file_addr);
          if (buf_length > p_modem->rx_packet.length)
          {
            buf_length = p_modem->rx_packet.length;
          }
          p_modem->file_buf_length = buf_length;
          p_modem->file_addr += buf_length;
          p_modem->file_received += buf_length;

          ymodemPutch(p_modem, YMODEM_ACK);
          p_modem->type = YMODEM_TYPE_DATA;
          ret = true;
        }
        break;

      case YMODEM_STATE_WAIT_LAST:
        ymodemPutch(p_modem, YMODEM_ACK);
        ymodemPutch(p_modem, YMODEM_C);
        p_modem->state = YMODEM_STATE_WAIT_END;
        break;

      case YMODEM_STATE_WAIT_END:
        ymodemPutch(p_modem, YMODEM_ACK);
        p_modem->state = YMODEM_STATE_WAIT_HEAD;
        p_modem->type = YMODEM_TYPE_END;
        ret = true;
        break;

      case YMODEM_STATE_WAIT_CANCEL:
        ymodemPutch(p_modem, YMODEM_ACK);
        p_modem->state = YMODEM_STATE_WAIT_HEAD;
        p_modem->type = YMODEM_TYPE_CANCEL;
        ret = true;
        break;
    }
  }
  else
  {
    if (p_modem->rx_packet.state == YMODEM_PACKET_WAIT_FIRST)
    {
      if (millis()-p_modem->pre_time >= p_modem->start_time)
      {
        p_modem->pre_time = millis();
        ymodemPutch(p_modem, YMODEM_C);
      }
    }
  }

  return ret;
}

bool ymodemReceivePacket(ymodem_packet_t *p_packet, uint8_t data_in)
{
  bool ret = false;


  switch(p_packet->state)
  {
    case YMODEM_PACKET_WAIT_FIRST:
      if (data_in == YMODEM_SOH)
      {
        p_packet->length = 128;
        p_packet->stx = data_in;
        p_packet->state = YMODEM_PACKET_WAIT_SEQ1;
      }
      if (data_in == YMODEM_STX)
      {
        p_packet->length = 1024;
        p_packet->stx = data_in;
        p_packet->state = YMODEM_PACKET_WAIT_SEQ1;
      }
      if (data_in == YMODEM_EOT)
      {
        p_packet->stx = data_in;
        ret = true;
      }
      if (data_in == YMODEM_CAN)
      {
        p_packet->stx = data_in;
        ret = true;
      }
      break;

    case YMODEM_PACKET_WAIT_SEQ1:
      p_packet->seq[0] = data_in;
      p_packet->state = YMODEM_PACKET_WAIT_SEQ2;
      break;

    case YMODEM_PACKET_WAIT_SEQ2:
      p_packet->seq[1] = data_in;
      if (p_packet->seq[0] == (uint8_t)(~data_in))
      {
        p_packet->index = 0;
        p_packet->state = YMODEM_PACKET_WAIT_DATA;
      }
      else
      {
        p_packet->state = YMODEM_PACKET_WAIT_FIRST;
      }
      break;

    case YMODEM_PACKET_WAIT_DATA:
      p_packet->data[p_packet->index] = data_in;
      p_packet->index++;
      if (p_packet->index >= p_packet->length)
      {
        p_packet->state = YMODEM_PACKET_WAIT_CRCH;
      }
      break;

    case YMODEM_PACKET_WAIT_CRCH:
      p_packet->crc_recv = (data_in<<8);
      p_packet->state = YMODEM_PACKET_WAIT_CRCL;
      break;

    case YMODEM_PACKET_WAIT_CRCL:
      p_packet->crc_recv |= (data_in<<0);
      p_packet->state = YMODEM_PACKET_WAIT_FIRST;

      p_packet->crc = crc16(p_packet->data, p_packet->length);

      if (p_packet->crc == p_packet->crc_recv)
      {
        ret = true;
      }
      //uartPrintf(_DEF_UART1, "crc %X %X\n", p_packet->crc, p_packet->crc_recv);
      break;
  }

  return ret;
}


#define CRC_POLY 0x1021

uint16_t crc_update(uint16_t crc_in, int incr)
{
  uint16_t xor = crc_in >> 15;
  uint16_t out = crc_in << 1;

  if (incr)
  {
    out++;
  }

  if (xor)
  {
    out ^= CRC_POLY;
  }

  return out;
}

uint16_t crc16(uint8_t *data, uint16_t size)
{
  uint16_t crc, i;

  for (crc = 0; size > 0; size--, data++)
  {
    for (i = 0x80; i; i >>= 1)
    {
      crc = crc_update(crc, *data & i);
    }
  }

  for (i = 0; i < 16; i++)
  {
    crc = crc_update(crc, 0);
  }

  return crc;
}


#ifdef _USE_HW_CLI
void cliYmodem(cli_args_t *args)
{
  bool ret = false;
  ymodem_t ymodem;
  bool keep_loop;
  uint8_t log_ch = _DEF_UART2;

  FRESULT fres;
  // "ymodem down" 명령어 체크
  if (args->argc == 1 && args->isStr(0, "down"))
  {
    // SD 카드가 마운트된 상태인지 확인

    fres = f_mount(&SDFatFs, SDPath, 0);
    if (fres != FR_OK)
    {
      cliPrintf("SD mount error: %d\n", fres);
      return;
    }

    // YMODEM 초기화 및 시작
    ymodemOpen(&ymodem, _DEF_UART1);
    keep_loop = true;
    cliPrintf("YMODEM start, waiting header...\n");

    while(keep_loop)
    {
      if (ymodemReceive(&ymodem) == true)
      {
        switch(ymodem.type)
        {
          case YMODEM_TYPE_START:
            // 파일명 및 크기 확보됨
            uartPrintf(log_ch, "YMODEM_TYPE_START %s %d\n", ymodem.file_name, ymodem.file_length);
            // FATFS로 파일 생성(덮어쓰기)
            fres = f_open(&yfile, ymodem.file_name,
                                      FA_CREATE_ALWAYS | FA_WRITE);
            if (fres != FR_OK)
            {
             cliPrintf("File open error: %d\n", fres);
             keep_loop = false;
            }
            break;

          case YMODEM_TYPE_DATA:
            // 패킷 버퍼에서 실제 데이터 길이만큼 쓰기
            //uartPrintf(log_ch, "YMODEM_TYPE_DATA %d %d %%\n", ymodem.rx_packet.seq[0], ymodem.file_received*100 / ymodem.file_length);
#ifndef _speed_up_test
            fres = f_write(&yfile,ymodem.rx_packet.data,ymodem.file_buf_length,&ybytes_written);
#else
            // → 버퍼링 최적화된 쓰기
            Stream_Write(ymodem.rx_packet.data, ymodem.file_buf_length);
#endif

            if (fres != FR_OK || ybytes_written != ymodem.file_buf_length)
            {
              cliPrintf("Write error: %d, wrote %u\n",
                        fres, ybytes_written);
              keep_loop = false;
            }
            else
            {
              // 진행률 표시
              cliPrintf("Data: %02u/%u (%3u%%)\n",
                        ymodem.rx_packet.seq[0],
                        ymodem.file_received,
                        (uint32_t)(ymodem.file_received * 100 / ymodem.file_length));
            }
            break;

          case YMODEM_TYPE_END:
#ifndef _speed_up_test
            // 전송 종료 → 파일 닫기
            f_close(&yfile);
#else
            // → 남은 스트림 데이터까지 한 번에 플러시
            Stream_Flush();
#endif
            cliPrintf("End: file saved\n");
            uartPrintf(log_ch, "YMODEM_TYPE_END \n");
            keep_loop = false;
            break;

          case YMODEM_TYPE_CANCEL:
#ifndef _speed_up_test
            // 전송 취소 시 닫고 삭제
             f_close(&yfile);
             f_unlink(ymodem.file_name);
#else
             // → 남은 스트림 데이터까지 한 번에 플러시
            Stream_Flush();
#endif



            cliPrintf("Canceled, file erased\n");
            uartPrintf(log_ch, "YMODEM_TYPE_CANCEL \n");
            keep_loop = false;
            break;

          case YMODEM_TYPE_ERROR:
            // 오류 시 닫고 삭제
            f_close(&yfile);
            f_unlink(ymodem.file_name);
            cliPrintf("Error, file erased\n");
            uartPrintf(log_ch, "YMODEM_TYPE_ERROR \n");
            keep_loop = false;
            break;
        }
      }
    }
    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("ymodem down\n");
  }
}

#endif
