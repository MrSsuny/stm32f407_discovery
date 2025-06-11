/*
 * qbuffer.c
 *
 *  Created on: Jun 11, 2025
 *      Author: 2005b
 */


#include "qbuffer.h"





void     qbufferInit(void)
{

}
bool     qbufferCreate(qbuffer_t *p_node, uint8_t *p_buf, uint32_t length)
{
  bool ret = true;
  p_node->in = 0;
  p_node->out = 0;
  p_node->len = length;
  p_node->p_buf = p_buf;
//  if(p_buf == NULL)
//  {
//    p_node->len = 0;
//    p_node0->p_buf = NULL;
//    ret = false;
//  }
  return ret;
}
bool     qbufferWrite(qbuffer_t *p_node, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint32_t next_in;
  for(int i=0; i<length; i++)
  {
    next_in = (p_node->in + 1) % p_node->len;
    if(next_in != p_node->out)
    {
      if(p_node->p_buf != NULL)
      {
        p_node->p_buf[p_node->in] = p_data[i];
      }
      p_node->in = next_in;
    }
    else
    {
      //buffer full
      ret = false;
      break;
    }
  }

  return ret;
}
bool qbufferRead(qbuffer_t *p_node, uint8_t *p_data, uint32_t length)
{
  bool ret = true;

  for(int i=0; i<length; i++)
  {
    if(p_node->p_buf != NULL)
    {
      p_data[i] = p_node->p_buf[p_node->out];
    }
    if(p_node->out != p_node->in)
    {
      p_node->out = (p_node->out + 1)  % p_node->len;
    }
    else
    {
      ret  = false;
      break;
    }
  }

  return ret;
}
uint32_t qbufferAvailable(qbuffer_t *p_node)
{
    return (p_node->in - p_node->out) % p_node->len;
}
void     qbufferFlush(qbuffer_t *p_node)
{
  p_node->in  = 0;
  p_node->out = 0;
}
