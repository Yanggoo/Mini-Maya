#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself
uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_View;        // The matrix that defines the camera's transformation.
uniform mat4 u_Proj;        // The matrix that defines the camera's projection.

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

in vec2 vs_UV;

out vec3 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec2 fs_UV;             // The UV of each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.

uniform mat4 uBindMatrix[100];
uniform mat4 uCurrentJointMatrix[100];
in ivec2 vs_JointIds;
in vec2 vs_JointFactors;

struct DualQuat {
    vec4 real;   // rotation (real part)
    vec4 dual;   // transformation (dual part)
};

// Function to convert a rotation matrix to a quaternion
vec4 mat3ToQuaternion(mat3 m) {
    float trace = m[0][0] + m[1][1] + m[2][2];
    float x, y, z, w;

    if (trace > 0.0) {
        float s = sqrt(trace + 1.0) * 2.0; // S=4*qw
        w = 0.25 * s;
        x = (m[2][1] - m[1][2]) / s;
        y = (m[0][2] - m[2][0]) / s;
        z = (m[1][0] - m[0][1]) / s;
    } else if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2])) {
        float s = sqrt(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2.0; // S=4*qx
        w = (m[2][1] - m[1][2]) / s;
        x = 0.25 * s;
        y = (m[0][1] + m[1][0]) / s;
        z = (m[0][2] + m[2][0]) / s;
    } else if (m[1][1] > m[2][2]) {
        float s = sqrt(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2.0; // S=4*qy
        w = (m[0][2] - m[2][0]) / s;
        x = (m[0][1] + m[1][0]) / s;
        y = 0.25 * s;
        z = (m[1][2] + m[2][1]) / s;
    } else {
        float s = sqrt(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2.0; // S=4*qz
        w = (m[1][0] - m[0][1]) / s;
        x = (m[0][2] + m[2][0]) / s;
        y = (m[1][2] + m[2][1]) / s;
        z = 0.25 * s;
    }

    return vec4(x, y, z, w);
}

// Function to convert a quaternion to a rotation matrix
mat3 quat_to_mat3(vec4 q) {
    float x = q.x, y = q.y, z = q.z, w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    mat3 m;
    m[0][0] = 1.0 - 2.0 * (yy + zz);
    m[0][1] = 2.0 * (xy - wz);
    m[0][2] = 2.0 * (xz + wy);

    m[1][0] = 2.0 * (xy + wz);
    m[1][1] = 1.0 - 2.0 * (xx + zz);
    m[1][2] = 2.0 * (yz - wx);

    m[2][0] = 2.0 * (xz - wy);
    m[2][1] = 2.0 * (yz + wx);
    m[2][2] = 1.0 - 2.0 * (xx + yy);

    return m;
}

// Function to multiply two quaternions
vec4 quat_mul(vec4 q1, vec4 q2) {
    return vec4(
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
    );
}

// Function to compute the conjugate of a quaternion
vec4 quat_conjugate(vec4 q) {
    return vec4(-q.xyz, q.w);
}

// Function to normalize a dual quaternion
DualQuat dualQuatNormalize(DualQuat dq) {
    float magnitude = length(dq.real);
    dq.real /= magnitude;
    dq.dual /= magnitude;
    return dq;
}

// Function to negate a dual quaternion
DualQuat dualQuatNegate(DualQuat dq) {
    dq.real = -dq.real;
    dq.dual = -dq.dual;
    return dq;
}

// Function to add two dual quaternions
DualQuat dualQuatAdd(DualQuat dq1, DualQuat dq2) {
    DualQuat result;
    result.real = dq1.real + dq2.real;
    result.dual = dq1.dual + dq2.dual;
    return result;
}

// Function to scale a dual quaternion by a scalar
DualQuat dualQuatScale(float s, DualQuat dq) {
    DualQuat result;
    result.real = dq.real * s;
    result.dual = dq.dual * s;
    return result;
}

// Function to rotate a vector using a quaternion
vec3 quat_rotate(vec4 q, vec3 v) {
    vec3 t = 2.0 * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}

// Function to convert a 4x4 transformation matrix to a dual quaternion
DualQuat Convert2DualQuat(mat4 m) {
    // Extract rotation matrix and translation vector
    mat3 rotMat = mat3(m);
    vec3 trans = m[3].xyz;

    // Convert rotation matrix to quaternion
    vec4 qr = mat3ToQuaternion(rotMat);

    // Create pure quaternion for translation
    vec4 qt = vec4(trans, 0.0);

    // Compute dual part
    vec4 qd = 0.5 * quat_mul(qt, qr);

    // Normalize the real part
    qr = normalize(qr);

    DualQuat dq;
    dq.real = qr;
    dq.dual = qd;

    return dq;
}

// Function to convert a dual quaternion to a 4x4 transformation matrix
mat4 Convert2Mat4(DualQuat dq) {
    // Normalize the dual quaternion
    dq = dualQuatNormalize(dq);

    // Extract rotation matrix from quaternion
    mat3 rotMat = quat_to_mat3(dq.real);

    // Compute translation vector
    vec4 qr_conj = quat_conjugate(dq.real);
    vec4 t_quat = quat_mul(dq.dual, qr_conj);
    vec3 trans = 2.0 * t_quat.xyz;

    // Build transformation matrix
    mat4 m = mat4(1.0);
    m[0] = vec4(rotMat[0], 0.0);
    m[1] = vec4(rotMat[1], 0.0);
    m[2] = vec4(rotMat[2], 0.0);
    m[3] = vec4(trans, 1.0);

    return m;
}


void main()
{
    fs_Col = vs_Col;
    fs_UV = vs_UV;    // Pass the vertex UVs to the fragment shader for interpolation


    mat4 bindMatrix1 = uBindMatrix[vs_JointIds.x];
    mat4 bindMatrix2 = uBindMatrix[vs_JointIds.y];

    mat4 currentMatrix1 = uCurrentJointMatrix[vs_JointIds.x];
    mat4 currentMatrix2 = uCurrentJointMatrix[vs_JointIds.y];

    currentMatrix1 = currentMatrix1*bindMatrix1;
    currentMatrix2 = currentMatrix2*bindMatrix2;

    DualQuat dualQuat1 = Convert2DualQuat(currentMatrix1);
    DualQuat dualQuat2 = Convert2DualQuat(currentMatrix2);

    // Retrieve weights
    float weight1 = vs_JointFactors.x;
    float weight2 = vs_JointFactors.y;

    // Ensure dual quaternions are in the same hemisphere
    if (dot(dualQuat1.real, dualQuat2.real) < 0.0) {
        dualQuat2 = dualQuatNegate(dualQuat2);
    }

    // Blend dual quaternions
    DualQuat blendedDualQuat = dualQuatAdd(
        dualQuatScale(weight1, dualQuat1),
        dualQuatScale(weight2, dualQuat2)
    );

    mat4 modelMatrix = Convert2Mat4(blendedDualQuat);
    // mat4 modelMatrix = currentMatrix1*weight1+currentMatrix2*weight2;
    modelMatrix = modelMatrix*u_Model;

    mat3 normalMatrix = inverse(transpose(mat3(modelMatrix)));
    fs_Nor = vec4(normalMatrix* vec3(vs_Nor), 0);

    vec4 modelposition = modelMatrix * vs_Pos;   // Temporarily store the transformed vertex positions for use below
    fs_Pos = modelposition.xyz;

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}
