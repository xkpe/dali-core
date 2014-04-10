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

#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <dali/dali.h>
#include <dali-test-suite-utils.h>

using namespace Dali;


void utc_dali_matrix_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_matrix_cleanup(void)
{
  test_return_value = TET_PASS;
}


int UtcDaliMatrixCtor(void)
{
  // Test initialized startup
  Matrix m1;

  float r1[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float r2[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  Matrix mr1(r1);
  Matrix mr2(r2);

  DALI_TEST_EQUALS(m1, mr1, 0.001f, TEST_LOCATION);

  // Test uninitialized startup
  // Stack construct a matrix to non zero, then stack construct another matrix over the top of it.
  float r3[] = { 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f};
  {
    Matrix m3(r3);
  }
  {
    Matrix m2(false);

    bool initialised = true;
    {
      float* els = m2.AsFloat();
      for(size_t idx=0; idx<16; ++idx, ++els)
      {
        if(*els != 0.0f)
          initialised = false;
      }
    }

    DALI_TEST_EQUALS(initialised, false, TEST_LOCATION);
  }

  Matrix m4(true);
  DALI_TEST_EQUALS(m4, mr1, 0.001f, TEST_LOCATION);

  m4 = m4;
  DALI_TEST_EQUALS(m4, mr1, 0.001f, TEST_LOCATION);

  Matrix m5(false);
  m5.SetIdentity();
  Matrix m6 = m5;
  DALI_TEST_EQUALS(m6, mr2, 0.001f, TEST_LOCATION);
  END_TEST;
}

// OrthoNormalise fixes floating point errors from matrix rotations
int UtcDaliMatrixOrthoNormalize0(void)
{
  Matrix m;
  m.SetIdentity();

  for (int i=0;i<1000;++i)
  {
    float f = i;
    Vector4 axis(cosf(f*0.001f), cosf(f*0.02f), cosf(f*0.03f), 0.0f);
    axis.Normalize();

    m.SetTransformComponents( Vector3::ONE, Quaternion(1.0f, axis), Vector3::ZERO );
    m.OrthoNormalize();
  }

  bool success = true;
  success &= fabsf(m.GetXAxis().Dot(m.GetYAxis())) < 0.001f;
  success &= fabsf(m.GetYAxis().Dot(m.GetXAxis())) < 0.001f;
  success &= fabsf(m.GetZAxis().Dot(m.GetYAxis())) < 0.001f;

  success &= fabsf(m.GetXAxis().Length() - 1.0f) < 0.001f;
  success &= fabsf(m.GetYAxis().Length() - 1.0f) < 0.001f;
  success &= fabsf(m.GetZAxis().Length() - 1.0f) < 0.001f;

  DALI_TEST_CHECK(success);
  END_TEST;
}

// OrthoNormalize is not flipping the axes and is maintaining the translation
int UtcDaliMatrixOrthoNormalize1(void)
{
  for (int i=0;i<1000;++i)
  {
    float f = i;
    Vector4 axis(cosf(f*0.001f), cosf(f*0.02f), cosf(f*0.03f), 0.0f);
    axis.Normalize();
    Vector3 center(10.0f, 15.0f, 5.0f);

    Matrix m0;
    m0.SetIdentity();
    m0.SetTransformComponents( Vector3::ONE, Quaternion(1.0f, axis), center );

    Matrix m1(m0);
    m1.OrthoNormalize();

    DALI_TEST_EQUALS(m0.GetXAxis(), m1.GetXAxis(), 0.001f, TEST_LOCATION);
    DALI_TEST_EQUALS(m0.GetYAxis(), m1.GetYAxis(), 0.001f, TEST_LOCATION);
    DALI_TEST_EQUALS(m0.GetZAxis(), m1.GetZAxis(), 0.001f, TEST_LOCATION);
    DALI_TEST_EQUALS(m0.GetTranslation(), m1.GetTranslation(), 0.001f, TEST_LOCATION);
  }
  END_TEST;
}

// Invert works
int UtcDaliMatrixInvert01(void)
{
  // We're going to invert a whole load of different matrices to make sure we don't
  // fail on particular orientations.
  for (int i=0;i<1000;++i)
  {
    float f = i;
    Vector4 axis(cosf(f*0.001f), cosf(f*0.02f), cosf(f*0.03f), 0.0f);
    axis.Normalize();
    Vector3 center(f, cosf(f) * 100.0f, cosf(f*0.5f) * 50.0f);

    Matrix m0;
    m0.SetIdentity();
    m0.SetTransformComponents( Vector3::ONE, Quaternion(1.0f, axis), center );

    Matrix m1(m0);
    m1.Invert();

    Matrix m2( false );
    Matrix::Multiply( m2, m0, m1 );

    DALI_TEST_EQUALS(m2, Matrix::IDENTITY, 0.001f, TEST_LOCATION);

    m1.Invert();    // doube invert - should be back to m0

    DALI_TEST_EQUALS(m0, m1, 0.001f, TEST_LOCATION);
  }
  END_TEST;
}


int UtcDaliMatrixInvert02(void)
{
  Matrix m1 = Matrix::IDENTITY;
  m1.SetXAxis(Vector3(0.0f, 0.0f, 0.0f));
  DALI_TEST_EQUALS(m1.Invert(), false, TEST_LOCATION);
  END_TEST;
}


// Invert transform works
int UtcDaliMatrixInvertTransform01(void)
{
  for (int i=0;i<1000;++i)
  {
    float f = i;
    Vector4 axis(cosf(f*0.001f), cosf(f*0.02f), cosf(f*0.03f), 0.0f);
    axis.Normalize();
    Vector3 center(f, cosf(f) * 100.0f, cosf(f*0.5f) * 50.0f);

    Matrix m0;
    m0.SetIdentity();
    m0.SetTransformComponents( Vector3::ONE, Quaternion(1.0f, axis), center );

    Matrix m1;
    m0.InvertTransform(m1);

    Matrix m2( false );
    Matrix::Multiply( m2, m0, m1 );

    DALI_TEST_EQUALS(m2, Matrix::IDENTITY, 0.001f, TEST_LOCATION);
  }
  END_TEST;
}


// Invert transform works
int UtcDaliMatrixInvertTransform02(void)
{
  std::string exceptionString( "EqualsZero( mMatrix[3] ) && EqualsZero( mMatrix[7] ) && EqualsZero( mMatrix[11] ) && Equals( mMatrix[15], 1.0f" );
  try
  {
    float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                    4.0f,  5.0f,  6.0f,  7.0f,
                    8.0f,  9.0f, 10.0f, 11.0f,
                    12.0f, 13.0f, 14.0f, 15.0f };
    Matrix m(els);

    Matrix it;
    m.InvertTransform(it);
    tet_result(TET_FAIL);
  }
  catch (Dali::DaliException& e)
  {
    tet_printf("Assertion %s failed at %s\n", e.mCondition.c_str(), e.mLocation.c_str());
    DALI_TEST_ASSERT( e, exceptionString, TEST_LOCATION );
  }

  try
  {
    float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                    4.0f,  5.0f,  6.0f,  7.0f,
                    8.0f,  9.0f, 10.0f, 11.0f,
                    12.0f, 13.0f, 14.0f, 15.0f };
    Matrix m(els);

    Matrix it;
    m.InvertTransform(it);
    tet_result(TET_FAIL);
  }
  catch (Dali::DaliException& e)
  {
    tet_printf("Assertion %s failed at %s\n", e.mCondition.c_str(), e.mLocation.c_str());
    DALI_TEST_ASSERT( e, exceptionString, TEST_LOCATION );
  }

  try
  {
    float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                    4.0f,  5.0f,  6.0f,  7.0f,
                    8.0f,  9.0f, 10.0f, 11.0f,
                    12.0f, 13.0f, 14.0f, 15.0f };
    Matrix m(els);

    Matrix it;
    m.InvertTransform(it);
    tet_result(TET_FAIL);
  }
  catch (Dali::DaliException& e)
  {
    tet_printf("Assertion %s failed at %s\n", e.mCondition.c_str(), e.mLocation.c_str());
    DALI_TEST_ASSERT( e, exceptionString, TEST_LOCATION );
  }

  try
  {
    float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                    4.0f,  5.0f,  6.0f,  7.0f,
                    8.0f,  9.0f, 10.0f, 11.0f,
                    12.0f, 13.0f, 14.0f, 15.0f };
    Matrix m(els);

    Matrix it;
    m.InvertTransform(it);
    tet_result(TET_FAIL);
  }
  catch (Dali::DaliException& e)
  {
    tet_printf("Assertion %s failed at %s\n", e.mCondition.c_str(), e.mLocation.c_str());
    DALI_TEST_ASSERT( e, exceptionString, TEST_LOCATION );
  }
  END_TEST;
}


// GetXAxis
int UtcDaliMatrixGetXAxis(void)
{
  float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                  4.0f,  5.0f,  6.0f,  7.0f,
                  8.0f,  9.0f, 10.0f, 11.0f,
                  12.0f, 13.0f, 14.0f, 15.0f };
  Matrix m(els);

  DALI_TEST_CHECK(m.GetXAxis() == Vector3(0.0f,  1.0f,  2.0f));
  END_TEST;
}

// GetYAxis
int UtcDaliMatrixGetYAxis(void)
{
  float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                  4.0f,  5.0f,  6.0f,  7.0f,
                  8.0f,  9.0f, 10.0f, 11.0f,
                  12.0f, 13.0f, 14.0f, 15.0f };
  Matrix m(els);

  DALI_TEST_CHECK(m.GetYAxis() == Vector3(4.0f,  5.0f,  6.0f));
  END_TEST;
}

// GetZAxis
int UtcDaliMatrixGetZAxis(void)
{
  float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                  4.0f,  5.0f,  6.0f,  7.0f,
                  8.0f,  9.0f, 10.0f, 11.0f,
                  12.0f, 13.0f, 14.0f, 15.0f };
  Matrix m(els);

  DALI_TEST_CHECK(m.GetZAxis() == Vector3(8.0f,  9.0f, 10.0f));
  END_TEST;
}

// GetTranslation
int UtcDaliMatrixGetTranslation(void)
{
  float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                  4.0f,  5.0f,  6.0f,  7.0f,
                  8.0f,  9.0f, 10.0f, 11.0f,
                  12.0f, 13.0f, 14.0f, 15.0f };
  Matrix m(els);

  DALI_TEST_EQUALS(m.GetTranslation(), Vector4(12.0f, 13.0f, 14.0f, 15.0f), TEST_LOCATION);
  END_TEST;
}

// GetTranslation
int UtcDaliMatrixGetTranslation3(void)
{
  float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                  4.0f,  5.0f,  6.0f,  7.0f,
                  8.0f,  9.0f, 10.0f, 11.0f,
                  12.0f, 13.0f, 14.0f, 15.0f };
  Matrix m(els);

  DALI_TEST_EQUALS(m.GetTranslation3(), Vector3(12.0f, 13.0f, 14.0f), TEST_LOCATION);
  END_TEST;
}

// SetIdentity
int UtcDaliMatrixSetIdentity(void)
{
  float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                  4.0f,  5.0f,  6.0f,  7.0f,
                  8.0f,  9.0f, 10.0f, 11.0f,
                  12.0f, 13.0f, 14.0f, 15.0f };
  Matrix m(els);
  m.SetIdentity();

  DALI_TEST_EQUALS(m, Matrix::IDENTITY, 0.001f, TEST_LOCATION);
  END_TEST;
}


int UtcDaliMatrixSetIdentityAndScale(void)
{
  float els[] = { 0.0f,  1.0f,  2.0f,  3.0f,
                  4.0f,  5.0f,  6.0f,  7.0f,
                  8.0f,  9.0f, 10.0f, 11.0f,
                  12.0f, 13.0f, 14.0f, 15.0f };
  Matrix m(els);
  m.SetIdentityAndScale(Vector3(4.0f, 4.0f, 4.0f));

  float els2[] = { 4.0f, 0.0f, 0.0f, 0.0f,
                   0.0f, 4.0f, 0.0f, 0.0f,
                   0.0f, 0.0f, 4.0f, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f };
  Matrix r(els2);

  DALI_TEST_EQUALS(m, r, 0.001f, TEST_LOCATION);
  END_TEST;
}


// SetXAxis
int UtcDaliMatrixSetXAxis(void)
{
  Matrix m;
  Vector3 v(2.0f, 3.0f, 4.0f);
  m.SetXAxis(v);

  DALI_TEST_CHECK(m.GetXAxis() == v);
  END_TEST;
}

// SetYAxis
int UtcDaliMatrixSetYAxis(void)
{
  Matrix m;
  Vector3 v(2.0f, 3.0f, 4.0f);
  m.SetYAxis(v);

  DALI_TEST_CHECK(m.GetYAxis() == v);
  END_TEST;
}

// SetZAxis
int UtcDaliMatrixSetZAxis(void)
{
  Matrix m;
  Vector3 v(2.0f, 3.0f, 4.0f);
  m.SetZAxis(v);

  DALI_TEST_CHECK(m.GetZAxis() == v);
  END_TEST;
}

// SetTranslation
int UtcDaliMatrixSetTranslation(void)
{
  Matrix m;
  Vector4 v(2.0f, 3.0f, 4.0f, 5.0f);
  m.SetTranslation(v);

  DALI_TEST_CHECK(m.GetTranslation() == v);
  END_TEST;
}

// SetTranslation
int UtcDaliMatrixSetTranslation3(void)
{
  Matrix m;
  Vector3 v(2.0f, 3.0f, 4.0f);
  m.SetTranslation(v);

  DALI_TEST_CHECK(m.GetTranslation3() == v);
  END_TEST;
}



// Transpose
int UtcDaliMatrixTranspose(void)
{
  float floats[] =
  {   0.0f,  1.0f,  2.0f,  3.0f,
      4.0f,  5.0f,  6.0f,  7.0f,
      8.0f,  9.0f, 10.0f,  11.0f,
     12.0f, 13.0f, 14.0f,  15.0f
  };

  Matrix m(floats);
  m.Transpose();

  bool success = true;

  for (int x=0;x<4;++x)
  {
    for (int y=0;y<4;++y)
    {
      success &= (m.AsFloat()[x+y*4] == floats[x*4+y]);
    }
  }

  DALI_TEST_CHECK(success);
  END_TEST;
}

int UtcDaliMatrixOStreamOperator(void)
{
  std::ostringstream oss;

  Matrix matrix;
  matrix.SetIdentity();

  oss << matrix;

  std::string expectedOutput = "[ [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1] ]";

  DALI_TEST_EQUALS( oss.str(), expectedOutput, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMatrixMultiply(void)
{
  Matrix m1 = Matrix::IDENTITY;

  float els[] = { 1.0f, 0.0f,    0.0f,   0.0f,
                  0.0f, 0.707f, 0.707f, 0.0f,
                  0.0f, -0.707f,  0.707f, 0.0f,
                  0.0f, 0.0f,    0.0f,   1.0f };
  Matrix result(els);

  Quaternion q(Radian(Degree(45.0f)), Vector3::XAXIS);
  Matrix m2(false);
  Matrix::Multiply(m2, m1, q);

  DALI_TEST_EQUALS(m2, result, 0.01f, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMatrixOperatorMultiply(void)
{
  Vector4 v1(2.0f, 5.0f, 4.0f, 0.0f);

  float els[] = {2.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 3.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 4.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f };
  Matrix m1(els);

  Vector4 v2 = m1 * v1;
  Vector4 r1(4.0f, 15.0f, 16.0f, 0.0f);
  DALI_TEST_EQUALS(v2, r1, 0.01f, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMatrixOperatorMultiply02(void)
{
  TestApplication application;

  Vector3 position ( 30.f, 40.f, 50.f);

  Matrix m1(false);
  m1.SetIdentity();
  m1.SetTranslation(-position);

  Vector4 positionV4(position);
  positionV4.w=1.0f;
  Vector4 output = m1 * positionV4;

  output.w = 0.0f;
  DALI_TEST_EQUALS(output, Vector4::ZERO, 0.01f, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMatrixOperatorEquals(void)
{
  Matrix m1 = Matrix::IDENTITY;

  float els[] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  Matrix r2(els);
  DALI_TEST_EQUALS(m1 == r2, true, TEST_LOCATION);

  float *f = m1.AsFloat();
  for(size_t i=0; i<16; i++)
  {
    f[15-i] = 1.2f;
    DALI_TEST_EQUALS(m1 == r2, false, TEST_LOCATION);
  }
  END_TEST;
}


int UtcDaliMatrixOperatorNotEquals(void)
{
  Matrix m1 = Matrix::IDENTITY;
  float els[] = {2.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 3.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 4.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f };
  Matrix r1(els);

  DALI_TEST_CHECK(m1 != r1);
  DALI_TEST_CHECK(!(m1 != m1));
  END_TEST;
}

int UtcDaliMatrixGetTransformComponents01(void)
{
  Matrix m2(Matrix::IDENTITY.AsFloat());
  Vector3 pos2;
  Vector3 scale2;
  Quaternion q2;
  m2.GetTransformComponents(pos2, q2, scale2);
  DALI_TEST_EQUALS(Vector3(0.0f, 0.0f, 0.0f), pos2, 0.001, TEST_LOCATION);
  DALI_TEST_EQUALS(Vector3(1.0f, 1.0f, 1.0f), scale2, 0.001, TEST_LOCATION);
  DALI_TEST_EQUALS(Quaternion(), q2, 0.001, TEST_LOCATION);
  END_TEST;
}


int UtcDaliMatrixGetTransformComponents02(void)
{
  // Create an arbitrary vector
  for( float x=-1.0f; x<=1.0f; x+=0.1f )
  {
    for( float y=-1.0f; y<1.0f; y+=0.1f )
    {
      for( float z=-1.0f; z<1.0f; z+=0.1f )
      {
        Vector3 vForward(x, y, z);
        vForward.Normalize();

        for( float angle = 5.0f; angle <= 360.0f; angle += 15.0f)
        {
          Quaternion rotation1(Radian(Degree(angle)), vForward);
          Vector3 scale1(2.0f, 3.0f, 4.0f);
          Vector3 position1(1.0f, 2.0f, 3.0f);

          Matrix m1(false);
          m1.SetTransformComponents(scale1, rotation1, position1);

          Vector3 position2;
          Quaternion rotation2;
          Vector3 scale2;
          m1.GetTransformComponents(position2, rotation2, scale2);

          DALI_TEST_EQUALS(position1, position2, 0.001, TEST_LOCATION);
          DALI_TEST_EQUALS(scale1, scale2, 0.001, TEST_LOCATION);
          DALI_TEST_EQUALS(rotation1, rotation2, 0.001, TEST_LOCATION);
        }
      }
    }
  }
  END_TEST;
}

int UtcDaliMatrixSetTransformComponents01(void)
{
  // Create an arbitrary vector
  for( float x=-1.0f; x<=1.0f; x+=0.1f )
  {
    for( float y=-1.0f; y<1.0f; y+=0.1f )
    {
      for( float z=-1.0f; z<1.0f; z+=0.1f )
      {
        Vector3 vForward(x, y, z);
        vForward.Normalize();

        for( float angle = 5.0f; angle <= 360.0f; angle += 15.0f)
        {
          Quaternion rotation1(Radian(Degree(angle)), vForward);

          Matrix m1(rotation1);
          Matrix result1(false);
          Vector4 vForward4(vForward.x, vForward.y, vForward.z, 0.0f);
          result1.SetTransformComponents( Vector3::ONE, Quaternion(Radian(Degree(angle)), vForward4), Vector3::ZERO );

          DALI_TEST_EQUALS(m1, result1, 0.001, TEST_LOCATION);

          Matrix m2(false);
          m2.SetTransformComponents(vForward, Quaternion::IDENTITY, Vector3::ZERO);

          Matrix result2a(Matrix::IDENTITY);
          result2a.SetXAxis(result2a.GetXAxis() * vForward[0]);
          result2a.SetYAxis(result2a.GetYAxis() * vForward[1]);
          result2a.SetZAxis(result2a.GetZAxis() * vForward[2]);

          DALI_TEST_EQUALS(m2, result2a, 0.001, TEST_LOCATION);

          Matrix m3(false);
          m3.SetTransformComponents(vForward, rotation1, Vector3::ZERO);

          Matrix result3(Matrix::IDENTITY);
          result3.SetXAxis(result3.GetXAxis() * vForward[0]);
          result3.SetYAxis(result3.GetYAxis() * vForward[1]);
          result3.SetZAxis(result3.GetZAxis() * vForward[2]);

          Matrix::Multiply(result3, result3, m1);
          DALI_TEST_EQUALS(m3, result3, 0.001, TEST_LOCATION);
        }
      }
    }
  }
  END_TEST;
}


int UtcDaliMatrixSetInverseTransformComponent01(void)
{
  // Create an arbitrary vector
  for( float x=-1.0f; x<=1.0f; x+=0.1f )
  {
    for( float y=-1.0f; y<1.0f; y+=0.1f )
    {
      for( float z=-1.0f; z<1.0f; z+=0.1f )
      {
        Vector3 vForward(x, y, z);
        vForward.Normalize();

        for( float angle = 5.0f; angle <= 360.0f; angle += 15.0f)
        {
          Quaternion rotation1(Radian(Degree(angle)), vForward);
          Vector3 scale1(2.0f, 3.0f, 4.0f);
          Vector3 position1(1.0f, 2.0f, 3.0f);

          Matrix m1(false);
          m1.SetTransformComponents(scale1, rotation1, position1);

          Matrix m2(false);
          m2.SetInverseTransformComponents(scale1, rotation1, position1);

          Matrix result;
          Matrix::Multiply(result, m1, m2);

          DALI_TEST_EQUALS(result, Matrix::IDENTITY, 0.001, TEST_LOCATION);
        }
      }
    }
  }
  END_TEST;
}

int UtcDaliMatrixSetInverseTransformComponent02(void)
{
  // Create an arbitrary vector
  for( float x=-1.0f; x<=1.0f; x+=0.1f )
  {
    for( float y=-1.0f; y<1.0f; y+=0.1f )
    {
      for( float z=-1.0f; z<1.0f; z+=0.1f )
      {
        Vector3 vForward(x, y, z);
        vForward.Normalize();

        for( float angle = 5.0f; angle <= 360.0f; angle += 15.0f)
        {
          Quaternion rotation1(Radian(Degree(angle)), vForward);
          Matrix rotationMatrix(rotation1);   // TEST RELIES ON THIS METHOD WORKING!!!

          Vector3 position1(5.0f, -6.0f, 7.0f);

          Matrix m1(false);
          m1.SetTransformComponents( Vector3::ONE, rotation1, position1 );

          Matrix m2(false);
          m2.SetInverseTransformComponents( rotationMatrix.GetXAxis(),
                                            rotationMatrix.GetYAxis(),
                                            rotationMatrix.GetZAxis(),
                                            position1 );

          Matrix result;
          Matrix::Multiply(result, m1, m2);

          DALI_TEST_EQUALS(result, Matrix::IDENTITY, 0.001, TEST_LOCATION);
        }
      }
    }
  }
  END_TEST;
}