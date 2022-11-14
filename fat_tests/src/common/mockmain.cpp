//
// Created by ddodd on 8/27/2021.
//

#include "gtest/gtest.h"
#include "EnvironmentProvider.h"

int main(int argc, char** argv) {
  // Handle any exceptions
  try {
    testing::InitGoogleTest(&argc, argv);
    DDIEnvironmentProvider environmentProvider(argc, argv);
    ::testing::Environment* environment = environmentProvider.GetEnvironment();
    if (environment != nullptr) {
        ::testing::AddGlobalTestEnvironment(environment);
        return RUN_ALL_TESTS();
    }
    // did not run the tests
    return 0;
  }
  catch (...){
    printf("Exception executing test \n");
    return -1;
  }
}

