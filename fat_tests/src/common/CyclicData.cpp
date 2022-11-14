//
// Created by ddodd on 8/27/2021.
//

    #include "CyclicData.h"
    AcontisCyclicData::AcontisCyclicData() {
    }
    AcontisCyclicData::AcontisCyclicData(void* input_pd, void* output_pd) {
       m_input_pd = (uint8_t *)input_pd;
       m_output_pd = (uint8_t *)output_pd;
    }
    void AcontisCyclicData::SetProcessData(uint8_t* output_data, unsigned int offset,  unsigned int length) {
      memcpy(&m_output_pd[offset], output_data, length);
    };
    int AcontisCyclicData::GetProcessData(unsigned int offset,  unsigned int length, uint8_t *ret_data, bool is_output_pd)  {
      memcpy(&ret_data[offset], &m_input_pd[offset], length);
      return 0;
    };
