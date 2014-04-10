//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// INTERNAL INCLUDES
#include <dali/dali.h>
#include <stdarg.h>

#include "dali-test-suite-utils.h"

using namespace Dali;

int test_return_value = TET_UNDEF;

void tet_result(int value)
{
  // First TET_PASS should set to zero
  // first TET_FAIL should prevent any further TET_PASS from setting back to zero
  // Any TET_FAIL should set to fail or leave as fail
  if( test_return_value != 1 )
    test_return_value = value;
}

#define END_TEST \
  return ((test_return_value>0)?1:0)


void tet_infoline(const char* str)
{
  fprintf(stderr, "%s\n", str);
}

void tet_printf(const char *format, ...)
{
  va_list arg;
  va_start(arg, format);
  vfprintf(stderr, format, arg);
  va_end(arg);
}

/**
 * DALI_TEST_CHECK is a wrapper for tet_result.
 * If the condition evaluates to false, then the function & line number is printed.
 * @param[in] The boolean expression to check
 */
#define DALI_TEST_CHECK(condition)                                                        \
if ( (condition) )                                                                        \
{                                                                                         \
  tet_result(TET_PASS);                                                                   \
}                                                                                         \
else                                                                                      \
{                                                                                         \
  fprintf(stderr, "%s Failed in %s at line %d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__);    \
  tet_result(TET_FAIL);                                                                   \
}

bool operator==(TimePeriod a, TimePeriod b)
{
  return Equals(a.durationSeconds, b.durationSeconds) && Equals(a.delaySeconds, b.delaySeconds) ;
}

std::ostream& operator<< (std::ostream& o, const TimePeriod value)
{
  return o << "( Duration:" << value.durationSeconds << " Delay:" << value.delaySeconds << ")";
}

void DALI_TEST_EQUALS( const Matrix3& matrix1, const Matrix3& matrix2, const char* location)
{
  const float* m1 = matrix1.AsFloat();
  const float* m2 = matrix2.AsFloat();
  bool equivalent = true;

  for (int i=0;i<9;++i)
  {
    equivalent &= (m1[i] != m2[i]);
  }

  if (!equivalent)
  {
    fprintf(stderr, "%s, checking\n"
               "(%f, %f, %f)    (%f, %f, %f)\n"
               "(%f, %f, %f) == (%f, %f, %f)\n"
               "(%f, %f, %f)    (%f, %f, %f)\n",
               location,
               m1[0],  m1[1], m1[2],   m2[0],  m2[1], m2[2],
               m1[3],  m1[4], m1[5],   m2[3],  m2[4], m2[5],
               m1[6],  m1[7], m1[8],   m2[6],  m2[7], m2[8]);

    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

void DALI_TEST_EQUALS( const Matrix3& matrix1, const Matrix3& matrix2, float epsilon, const char* location)
{
  const float* m1 = matrix1.AsFloat();
  const float* m2 = matrix2.AsFloat();
  bool equivalent = true;

  for (int i=0;i<9;++i)
  {
    equivalent &= (fabsf(m1[i] - m2[i])<epsilon);
  }

  if (!equivalent)
  {
    fprintf(stderr, "%s, checking\n"
               "(%f, %f, %f)    (%f, %f, %f)\n"
               "(%f, %f, %f) == (%f, %f, %f)\n"
               "(%f, %f, %f)    (%f, %f, %f)\n",
               location,
               m1[0],  m1[1], m1[2],   m2[0],  m2[1], m2[2],
               m1[3],  m1[4], m1[5],   m2[3],  m2[4], m2[5],
               m1[6],  m1[7], m1[8],   m2[6],  m2[7], m2[8]);

    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

void DALI_TEST_EQUALS( const Matrix& matrix1, const Matrix& matrix2, const char* location)
{
  const float* m1 = matrix1.AsFloat();
  const float* m2 = matrix2.AsFloat();
  bool identical = true;

  int i;
  for (i=0;i<16;++i)
  {
    if(m1[i] != m2[i])
    {
      identical = false;
      break;
    }
  }

  if (!identical)
  {
    fprintf(stderr, "%s, checking\n"
               "(%f, %f, %f, %f)    (%f, %f, %f, %f)\n"
               "(%f, %f, %f, %f) == (%f, %f, %f, %f)\n"
               "(%f, %f, %f, %f)    (%f, %f, %f, %f)\n"
               "(%f, %f, %f, %f)    (%f, %f, %f, %f)\n", location,
               m1[0],  m1[1],  m1[2],  m1[3],   m2[0],  m2[1],  m2[2],  m2[3],
               m1[4],  m1[5],  m1[6],  m1[7],   m2[4],  m2[5],  m2[6],  m2[7],
               m1[8],  m1[9], m1[10], m1[11],   m2[8],  m2[9], m2[10], m2[11],
              m1[12], m1[13], m1[14], m1[15],  m2[12], m2[13], m2[14], m2[15]);

    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

void DALI_TEST_EQUALS( const Matrix& matrix1, const Matrix& matrix2, float epsilon, const char* location)
{
  const float* m1 = matrix1.AsFloat();
  const float* m2 = matrix2.AsFloat();
  bool equivalent = true;

  for (int i=0;i<16;++i)
  {
    equivalent &= (fabsf(m1[i] - m2[i])<epsilon);
  }

  if (!equivalent)
  {
    fprintf(stderr, "%s, checking\n"
               "(%f, %f, %f, %f)    (%f, %f, %f, %f)\n"
               "(%f, %f, %f, %f) == (%f, %f, %f, %f)\n"
               "(%f, %f, %f, %f)    (%f, %f, %f, %f)\n"
               "(%f, %f, %f, %f)    (%f, %f, %f, %f)\n", location,
               m1[0],  m1[1],  m1[2],  m1[3],   m2[0],  m2[1],  m2[2],  m2[3],
               m1[4],  m1[5],  m1[6],  m1[7],   m2[4],  m2[5],  m2[6],  m2[7],
               m1[8],  m1[9], m1[10], m1[11],   m2[8],  m2[9], m2[10], m2[11],
              m1[12], m1[13], m1[14], m1[15],  m2[12], m2[13], m2[14], m2[15]);

    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}


/**
 * Test whether two strings are equal.
 * @param[in] str1 The first string
 * @param[in] str2 The second string
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const std::string &str1, const char* str2, const char* location)
{
  DALI_TEST_EQUALS(str1.c_str(), str2, location);
}

/**
 * Test whether two strings are equal.
 * @param[in] str1 The first string
 * @param[in] str2 The second string
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const char* str1, const std::string &str2, const char* location)
{
  DALI_TEST_EQUALS(str1, str2.c_str(), location);
}


/**
 * Test whether one unsigned integer value is greater than another.
 * Test succeeds if value1 > value2
 * @param[in] value1 The first value
 * @param[in] value2 The second value
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_GREATER(unsigned int value1, unsigned int value2, const char* location)
{
  if (!(value1 > value2))
  {
    fprintf(stderr, "%s, checking %d > %d\n", location, value1, value2);
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

/**
 * Test whether one float value is greater than another.
 * Test succeeds if value1 > value2
 * @param[in] value1 The first value
 * @param[in] value2 The second value
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_GREATER( float value1, float value2, const char* location)
{
  if (!(value1 > value2))
  {
    fprintf(stderr, "%s, checking %f > %f\n", location, value1, value2);
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

/**
 * Test whether the assertion condition that failed and thus triggered the
 * exception \b e contained a given substring at the start of its literal text.
 * @param[in] e The exception that we expect was fired by a runtime assertion
 *              failure.
 * @param[in] conditionSubString The text that we expect to be present in an
 *                               assertion which triggered the exception.
 * @param[in] location The TEST_LOCATION macro should be used here.
 *
 * @remark **Side-effects:** The result of the tet test is set to TET_PASS if
 *         the substring is at the start of the exception's condition and
 *         TET_FAIL if it isn't. Note, if the result of a test is set multiple
 *         times, a TET_FAIL will override any number of TET_PASSes.
 */
void DALI_TEST_ASSERT( DaliException& e, std::string conditionSubString, const char* location )
{
  if( 0u != e.mCondition.find( conditionSubString ))
  {
    fprintf(stderr, "Assertion %s failed at %s\n", conditionSubString.c_str(), location);
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

/** Self-documenting wrapper for DALI_TEST_ASSERT.
 * @copydoc DALI_TEST_ASSERT()
 */
void DALI_TEST_ASSERT_CONDITION_STARTS_WITH_SUBSTRING( DaliException& exceptionFromAssertion, std::string conditionSubString, const char* location )
{
  DALI_TEST_ASSERT(exceptionFromAssertion, conditionSubString, location);
}

// Functor to test whether an Applied signal is emitted
ConstraintAppliedCheck::ConstraintAppliedCheck( bool& signalReceived )
: mSignalReceived( signalReceived )
{
}

void ConstraintAppliedCheck::operator()( ActiveConstraint& constraint )
{
  mSignalReceived = true;
}

void ConstraintAppliedCheck::Reset()
{
  mSignalReceived = false;
}

void ConstraintAppliedCheck::CheckSignalReceived()
{
  if ( !mSignalReceived )
  {
    fprintf(stderr,  "Expected Applied signal was not received\n" );
    tet_result( TET_FAIL );
  }
  else
  {
    tet_result( TET_PASS );
  }
}

void ConstraintAppliedCheck::CheckSignalNotReceived()
{
  if ( mSignalReceived )
  {
    fprintf(stderr,  "Unexpected Applied signal was received\n" );
    tet_result( TET_FAIL );
  }
  else
  {
    tet_result( TET_PASS );
  }
}

BitmapImage CreateBitmapImage()
{
  BitmapImage image = BitmapImage::New(4,4,Pixel::RGBA8888);

  PixelBuffer* pixbuf = image.GetBuffer();

  // Using a 4x4 image gives a better blend with the GL implementation
  // than a 3x3 image
  for(size_t i=0; i<16; i++)
  {
    pixbuf[i*4+0] = 0xFF;
    pixbuf[i*4+1] = 0xFF;
    pixbuf[i*4+2] = 0xFF;
    pixbuf[i*4+3] = 0xFF;
  }

  return image;
}