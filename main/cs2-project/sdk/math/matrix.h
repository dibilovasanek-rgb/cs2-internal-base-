#pragma once
#include "vector.h"
#include <span>
#include <limits>
#include <DirectXMath.h>

struct cplane_t
{
    vec3_t normal;
    float dist;
    uint8_t type;   // for fast side tests
    uint8_t signbits;  // signx + (signy<<1) + (signz<<1)
    uint8_t pad[2];
};

class matrix2x4_t {
public:
    vec3_t get_origin() {
        return vec3_t(_11, _12, _13);
    }

    void set_origin(int nIndex, vec3_t vecValue) {
        this[nIndex]._11 = vecValue.x;
        this[nIndex]._12 = vecValue.y;
        this[nIndex]._13 = vecValue.z;
    }

    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
        };
    };
};

class matrix3x4_t
{
public:
    matrix3x4_t() {}
    matrix3x4_t(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23)
    {
        m_flMatVal[0][0] = m00; m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
        m_flMatVal[1][0] = m10; m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
        m_flMatVal[2][0] = m20; m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
    }
    //-----------------------------------------------------------------------------
    // Creates a aMatrix where the x axis = forward
    // the y axis = left, and the z axis = up
    //-----------------------------------------------------------------------------
    void Init(const vec3_t& xAxis, const vec3_t& yAxis, const vec3_t& zAxis, const vec3_t& vecOrigin)
    {
        m_flMatVal[0][0] = xAxis.x; m_flMatVal[0][1] = yAxis.x; m_flMatVal[0][2] = zAxis.x; m_flMatVal[0][3] = vecOrigin.x;
        m_flMatVal[1][0] = xAxis.y; m_flMatVal[1][1] = yAxis.y; m_flMatVal[1][2] = zAxis.y; m_flMatVal[1][3] = vecOrigin.y;
        m_flMatVal[2][0] = xAxis.z; m_flMatVal[2][1] = yAxis.z; m_flMatVal[2][2] = zAxis.z; m_flMatVal[2][3] = vecOrigin.z;
    }

    //-----------------------------------------------------------------------------
    // Creates a aMatrix where the x axis = forward
    // the y axis = left, and the z axis = up
    //-----------------------------------------------------------------------------
    matrix3x4_t(const vec3_t& xAxis, const vec3_t& yAxis, const vec3_t& zAxis, const vec3_t& vecOrigin)
    {
        Init(xAxis, yAxis, zAxis, vecOrigin);
    }

    constexpr void SetForward(const vec3_t& vecForward)
    {
        this->m_flMatVal[0][0] = vecForward.x;
        this->m_flMatVal[1][0] = vecForward.y;
        this->m_flMatVal[2][0] = vecForward.z;
    }

    constexpr void SetLeft(const vec3_t& vecLeft)
    {
        this->m_flMatVal[0][1] = vecLeft.x;
        this->m_flMatVal[1][1] = vecLeft.y;
        this->m_flMatVal[2][1] = vecLeft.z;
    }

    constexpr void SetUp(const vec3_t& vecUp)
    {
        this->m_flMatVal[0][2] = vecUp.x;
        this->m_flMatVal[1][2] = vecUp.y;
        this->m_flMatVal[2][2] = vecUp.z;
    }

    constexpr void SetOrigin(const vec3_t& vecOrigin)
    {
        this->m_flMatVal[0][3] = vecOrigin.x;
        this->m_flMatVal[1][3] = vecOrigin.y;
        this->m_flMatVal[2][3] = vecOrigin.z;
    }
    inline void Invalidate(void)
    {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; j++) {
                m_flMatVal[i][j] = std::numeric_limits<float>::infinity();;
            }
        }
    }

    inline void MatrixSetColumn(const vec3_t& in, int column)
    {
        m_flMatVal[0][column] = in.x;
        m_flMatVal[1][column] = in.y;
        m_flMatVal[2][column] = in.z;
    }

    /*
    void QuaternionMatrix(const Quaternion& q)
    {
        m_flMatVal[0][0] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
        m_flMatVal[1][0] = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
        m_flMatVal[2][0] = 2.0 * q.x * q.z - 2.0 * q.w * q.y;

        m_flMatVal[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
        m_flMatVal[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
        m_flMatVal[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

        m_flMatVal[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
        m_flMatVal[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
        m_flMatVal[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

        m_flMatVal[0][3] = 0.0f;
        m_flMatVal[1][3] = 0.0f;
        m_flMatVal[2][3] = 0.0f;
    }

    void QuaternionMatrix(const Quaternion& q, const vec3_t& pos)
    {
        QuaternionMatrix(q);

        m_flMatVal[0][3] = pos.x;
        m_flMatVal[1][3] = pos.y;
        m_flMatVal[2][3] = pos.z;
    }

    matrix3x4_t contact_transforms(matrix3x4_t in) const
    {
        matrix3x4_t out = {};

        out.m_flMatVal[0][0] = m_flMatVal[0][0] * in.m_flMatVal[0][0] + m_flMatVal[0][1] * in.m_flMatVal[1][0] + m_flMatVal[0][2] * in.m_flMatVal[2][0];
        out.m_flMatVal[0][1] = m_flMatVal[0][0] * in.m_flMatVal[0][1] + m_flMatVal[0][1] * in.m_flMatVal[1][1] + m_flMatVal[0][2] * in.m_flMatVal[2][1];
        out.m_flMatVal[0][2] = m_flMatVal[0][0] * in.m_flMatVal[0][2] + m_flMatVal[0][1] * in.m_flMatVal[1][2] + m_flMatVal[0][2] * in.m_flMatVal[2][2];
        out.m_flMatVal[0][3] = m_flMatVal[0][0] * in.m_flMatVal[0][3] + m_flMatVal[0][1] * in.m_flMatVal[1][3] + m_flMatVal[0][2] * in.m_flMatVal[2][3] + m_flMatVal[0][3];
        out.m_flMatVal[1][0] = m_flMatVal[1][0] * in.m_flMatVal[0][0] + m_flMatVal[1][1] * in.m_flMatVal[1][0] + m_flMatVal[1][2] * in.m_flMatVal[2][0];
        out.m_flMatVal[1][1] = m_flMatVal[1][0] * in.m_flMatVal[0][1] + m_flMatVal[1][1] * in.m_flMatVal[1][1] + m_flMatVal[1][2] * in.m_flMatVal[2][1];
        out.m_flMatVal[1][2] = m_flMatVal[1][0] * in.m_flMatVal[0][2] + m_flMatVal[1][1] * in.m_flMatVal[1][2] + m_flMatVal[1][2] * in.m_flMatVal[2][2];
        out.m_flMatVal[1][3] = m_flMatVal[1][0] * in.m_flMatVal[0][3] + m_flMatVal[1][1] * in.m_flMatVal[1][3] + m_flMatVal[1][2] * in.m_flMatVal[2][3] + m_flMatVal[1][3];
        out.m_flMatVal[2][0] = m_flMatVal[2][0] * in.m_flMatVal[0][0] + m_flMatVal[2][1] * in.m_flMatVal[1][0] + m_flMatVal[2][2] * in.m_flMatVal[2][0];
        out.m_flMatVal[2][1] = m_flMatVal[2][0] * in.m_flMatVal[0][1] + m_flMatVal[2][1] * in.m_flMatVal[1][1] + m_flMatVal[2][2] * in.m_flMatVal[2][1];
        out.m_flMatVal[2][2] = m_flMatVal[2][0] * in.m_flMatVal[0][2] + m_flMatVal[2][1] * in.m_flMatVal[1][2] + m_flMatVal[2][2] * in.m_flMatVal[2][2];
        out.m_flMatVal[2][3] = m_flMatVal[2][0] * in.m_flMatVal[0][3] + m_flMatVal[2][1] * in.m_flMatVal[1][3] + m_flMatVal[2][2] * in.m_flMatVal[2][3] + m_flMatVal[2][3];

        return out;
    }
    */
    vec3_t GetXAxis()  const { return at(0); }
    vec3_t GetYAxis()  const { return at(1); }
    vec3_t GetZAxis()  const { return at(2); }
    vec3_t GetOrigin() const { return at(3); }

    vec3_t at(int i) const { return vec3_t{ m_flMatVal[0][i], m_flMatVal[1][i], m_flMatVal[2][i] }; }

    float* operator[](int i) { return m_flMatVal[i]; }
    const float* operator[](int i) const { return m_flMatVal[i]; }
    float* Base() { return &m_flMatVal[0][0]; }
    const float* Base() const { return &m_flMatVal[0][0]; }

    float m_flMatVal[3][4];
};

class VMatrix
{
public:

    VMatrix();
    VMatrix(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    );

    // Creates a aMatrix where the x axis = forward
    // the y axis = left, and the z axis = up
    VMatrix(const vec3_t& forward, const vec3_t& left, const vec3_t& up);

    // Construct from a 3x4 aMatrix
    VMatrix(const matrix3x4_t& matrix3x4);

    // Set the values in the aMatrix.
    void  Init(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    );



    // Initialize from a 3x4
    void  Init(const matrix3x4_t& matrix3x4);

    // array access
    inline float* operator[](int i)
    {
        return m[i];
    }

    inline const float* operator[](int i) const
    {
        return m[i];
    }

    // Get a pointer to m[ 0 ][ 0 ]
    inline float* Base()
    {
        return &m[0][0];
    }

    inline const float* Base() const
    {
        return &m[0][0];
    }

    void  SetLeft(const vec3_t& vLeft);
    void  SetUp(const vec3_t& vUp);
    void  SetForward(const vec3_t& vForward);

    void  GetBasisVectors(vec3_t& vForward, vec3_t& vLeft, vec3_t& vUp) const;
    void  SetBasisVectors(const vec3_t& vForward, const vec3_t& vLeft, const vec3_t& vUp);

    // Get/Set the translation.
    vec3_t& GetTranslation(vec3_t& vTrans) const;
    void  SetTranslation(const vec3_t& vTrans);

    void  PreTranslate(const vec3_t& vTrans);
    void  PostTranslate(const vec3_t& vTrans);

    matrix3x4_t& As3x4();
    const matrix3x4_t& As3x4() const;
    void  CopyFrom3x4(const matrix3x4_t& m3x4);
    void  Set3x4(matrix3x4_t& matrix3x4) const;

    bool  operator==(const VMatrix& src) const;
    bool  operator!=(const VMatrix& src) const { return !(*this == src); }

    // Access the basis vectors.
    vec3_t  GetLeft() const;
    vec3_t  GetUp() const;
    vec3_t  GetForward() const;
    vec3_t  GetTranslation() const;


    // aMatrix->vec3_t operations.
public:
    // Multiply by a 3D vec3_t (same as operator*).
    void  V3Mul(const vec3_t& vIn, vec3_t& vOut) const;

    // Multiply by a 4D vec3_t.
    //void  V4Mul( const Vector4D &vIn, Vector4D &vOut ) const;

    // Applies the rotation (ignores translation in the aMatrix). (This just calls VMul3x3).
    vec3_t  ApplyRotation(const vec3_t& vVec) const;

    // Multiply by a vec3_t (divides by w, assumes input w is 1).
    vec3_t  operator*(const vec3_t& vVec) const;

    // Multiply by the upper 3x3 part of the aMatrix (ie: only apply rotation).
    vec3_t  VMul3x3(const vec3_t& vVec) const;

    // Apply the inverse (transposed) rotation (only works on pure rotation aMatrix)
    vec3_t  VMul3x3Transpose(const vec3_t& vVec) const;

    // Multiply by the upper 3 rows.
    vec3_t  VMul4x3(const vec3_t& vVec) const;

    // Apply the inverse (transposed) transformation (only works on pure rotation/translation)
    vec3_t  VMul4x3Transpose(const vec3_t& vVec) const;


    // aMatrix->plane operations.
    //public:
    // Transform the plane. The aMatrix can only contain translation and rotation.
    //void  TransformPlane( const VPlane &inPlane, VPlane &outPlane ) const;

    // Just calls TransformPlane and returns the result.
    //VPlane  operator*(const VPlane &thePlane) const;

    // aMatrix->aMatrix operations.
public:

    VMatrix& operator=(const VMatrix& mOther);

    // Add two matrices.
    const VMatrix& operator+=(const VMatrix& other);

    // Add/Subtract two matrices.
    VMatrix  operator+(const VMatrix& other) const;
    VMatrix  operator-(const VMatrix& other) const;

    // Negation.
    VMatrix  operator-() const;

    // Return inverse aMatrix. Be careful because the results are undefined 
    // if the aMatrix doesn't have an inverse (ie: InverseGeneral returns false).
    VMatrix  operator~() const;

    // aMatrix operations.
public:
    // Set to identity.
    void  Identity();
    bool  IsIdentity() const;
public:
    // The aMatrix.
    float  m[4][4];
};

inline void MatrixGetColumn(const matrix3x4_t& src, int nCol, vec3_t& pColumn)
{
    pColumn.x = src[0][nCol];
    pColumn.y = src[1][nCol];
    pColumn.z = src[2][nCol];
}

inline void MatrixPosition(const matrix3x4_t& aMatrix, vec3_t& position)
{
    MatrixGetColumn(aMatrix, 3, position);
}

inline void MatrixAngles(const matrix3x4_t& aMatrix, float* angAngles)
{
    float forward[3];
    float left[3];
    float up[3];

    //
    // Extract the basis vectors from the aMatrix. Since we only need the z
    // component of the up vec3_t, we don't get x and y.
    //
    forward[0] = aMatrix[0][0];
    forward[1] = aMatrix[1][0];
    forward[2] = aMatrix[2][0];
    left[0] = aMatrix[0][1];
    left[1] = aMatrix[1][1];
    left[2] = aMatrix[2][1];
    up[2] = aMatrix[2][2];

    float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

    // enough here to get angAngles?
    if (xyDist > 0.001f)
    {
        // (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the x axis
        angAngles[1] = DirectX::XMConvertToDegrees(atan2f(forward[1], forward[0]));

        // (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
        angAngles[0] = DirectX::XMConvertToDegrees(atan2f(-forward[2], xyDist));

        // (roll)	z = ATAN( left.z, up.z );
        angAngles[2] = DirectX::XMConvertToDegrees(atan2f(left[2], up[2]));
    }
    else	// forward is mostly z, gimbal lock-
    {
        // (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
        angAngles[1] = DirectX::XMConvertToDegrees(atan2f(-left[0], left[1]));

        // (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
        angAngles[0] = DirectX::XMConvertToDegrees(atan2f(-forward[2], xyDist));

        // Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
        angAngles[2] = 0;
    }
}

inline void MatrixAngles(const matrix3x4_t& aMatrix, vec3_t& angAngles)
{
    MatrixAngles(aMatrix, &angAngles.x);
}

inline void MatrixAngles(const matrix3x4_t& aMatrix, vec3_t& angAngles, vec3_t& vecPosition)
{
    MatrixAngles(aMatrix, angAngles);
    MatrixPosition(aMatrix, vecPosition);
}

class v_matrix {
public:
    auto operator[](int i) const { return m[i]; }

    float m[4][4];
};
