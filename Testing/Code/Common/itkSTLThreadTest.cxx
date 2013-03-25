/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkSTLThreadTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkMultiThreader.h"
#include <list>
#include <stdlib.h>

namespace itkSTLThreadTestImpl
{
int done = 0;
int numberOfIterations = 10;

ITK_THREAD_RETURN_TYPE Runner(void*);
int Thread(int);
} // namespace itkSTLThreadTestImpl

int itkSTLThreadTest(int argc, char* argv[])
{
// On some old sgi compilers, this test gets into an infinite loop.
#if defined(__sgi) && defined(_COMPILER_VERSION) && _COMPILER_VERSION <= 730
  return EXIT_SUCCESS;
#else
  // Choose a number of threads.
  int numThreads = 10;
  if(argc > 1)
    {
    int nt = atoi(argv[1]);
    if(nt > 1)
      {
      numThreads = nt;
      }
    }
  
  // Choose a number of iterations (0 is infinite).
  if(argc > 2)
    {
    int ni = atoi(argv[2]);
    if(ni >= 0)
      {
      itkSTLThreadTestImpl::numberOfIterations = ni;
      }
    }
  
  // Enforce limit on number of threads.
  if(numThreads > ITK_MAX_THREADS)
    {
    numThreads = ITK_MAX_THREADS;
    }
  
  // Report what we'll do.
  std::cout << "Using " << numThreads << " threads.\n";
  if(itkSTLThreadTestImpl::numberOfIterations)
    {
    std::cout << "Using " << itkSTLThreadTestImpl::numberOfIterations
              << " iterations.\n";
  
    }
  else
    {
    std::cout << "Using infinite iterations.\n";
    }
  
  // Create result array.  Assume failure.
  int* results = new int[numThreads];
  int i;
  for(i=0; i < numThreads; ++i)
    {
    results[i] = 0;
    }
  
  // Create and execute the threads.
  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetSingleMethod(itkSTLThreadTestImpl::Runner, results);  
  threader->SetNumberOfThreads(numThreads);
  threader->SingleMethodExecute();
  
  // Report results.
  int result = 0;
  for(i=0; i < numThreads; ++i)
    {
    if(!results[i])
      {
      std::cerr << "Thread " << i << " failed." << std::endl;
      result = 1;
      }
    }
  delete [] results;

  // Test other methods for coverage.
  std::cout << "Done with primary test.  Testing more methods..."
            << std::endl;
  threader->SetGlobalMaximumNumberOfThreads(1);
  threader->SetGlobalDefaultNumberOfThreads(1);
  
  std::cout << "threader->GetGlobalMaximumNumberOfThreads(): "
            << threader->GetGlobalMaximumNumberOfThreads() << std::endl;
  
  int threadId = threader->SpawnThread(itkSTLThreadTestImpl::Runner, 0);
  std::cout << "SpawnThread(itkSTLThreadTestImpl::Runner, results): "
            << threadId << std::endl;
  threader->TerminateThread(threadId);
  std::cout << "Spawned thread terminated." << std::endl;
  
  return result;
#endif
}

namespace itkSTLThreadTestImpl
{

ITK_THREAD_RETURN_TYPE Runner(void* infoIn)
{
  // Get the thread id and result pointer and run the method for this
  // thread.
  itk::MultiThreader::ThreadInfoStruct* info =
    static_cast<itk::MultiThreader::ThreadInfoStruct*>(infoIn);
  int tnum = info->ThreadID;
  int* results = static_cast<int*>(info->UserData);
  if(results)
    {
    results[tnum] = itkSTLThreadTestImpl::Thread(tnum);
    }
  else
    {
    itkSTLThreadTestImpl::Thread(tnum);
    }
#ifndef ITK_USE_SPROC
  return EXIT_SUCCESS;
#endif
}

int Thread(int tnum)
{
  // Implementation in individual thread.  We don't care about
  // mutexing the output because it doesn't matter for the test.
  std::cout << "Starting " << tnum << "\n";
  
  // Create a list with which to play.
  std::list<int> l;
  
  // Choose a size for each iteration for this thread.
  int count = 10000+100*tnum;
  
  int iteration=0;
  while(!done && !(numberOfIterations && (iteration >= numberOfIterations)))
    {
    // Output progress of this thread.
    std::cout << tnum << ": " << iteration << "\n";
    
    // Fill the list.
    int j;
    for(j=0; j < count; ++j)
      {
      l.push_back(j);
      }
    
    // Empty the list while making sure values match.  Threading
    // errors can cause mismatches here, which is the purpose of the
    // test.
    for(j=0; j < count; ++j)
      {
      if(l.front() != j)
        {
        std::cerr << "Mismatch in thread " << tnum << "!\n";
        done = 1;
        }
      l.pop_front();
      }
    
    ++iteration;
    }
  
  // Only get here on failure or iterations finished.
  if(numberOfIterations && (iteration >= numberOfIterations))
    {
    // Success.
    return EXIT_FAILURE;
    }
  else
    {
    // Failure.
    return EXIT_SUCCESS;
    }
}

} // namespace itkSTLThreadTestImpl