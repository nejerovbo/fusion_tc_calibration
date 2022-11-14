//
// Created by ddodd on 8/27/2021.
//

#ifndef FATPARENT_TRAIN_H
#define FATPARENT_TRAIN_H

#include <stdint.h>
#include <string.h>

class CyclicData  {
public:
    virtual void SetProcessData(uint8_t* output_data, unsigned int offset,  unsigned int length) = 0;
    virtual int GetProcessData(unsigned int offset,  unsigned int length,  uint8_t *ret_data, bool is_output_pd = false) = 0;
};

class AcontisCyclicData : public CyclicData {

    uint8_t  *m_input_pd;
    uint8_t  *m_output_pd;
public:
    AcontisCyclicData() ;
    AcontisCyclicData(void* input_pd, void* output_pd) ;
    void SetProcessData(uint8_t* output_data, unsigned int offset,  unsigned int length);
    int GetProcessData( unsigned int offset,  unsigned int length,  uint8_t *ret_data, bool is_output_pd = false);
};

#endif //FATPARENT_TRAIN_H


