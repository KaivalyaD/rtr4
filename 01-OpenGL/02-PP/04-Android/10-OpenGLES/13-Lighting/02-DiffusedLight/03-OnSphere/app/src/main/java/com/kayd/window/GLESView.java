package com.kayd.window;

import android.opengl.GLSurfaceView;  // for GLSurfaceView
import javax.microedition.khronos.opengles.GL10;  // for GL10

import javax.microedition.khronos.egl.EGLConfig;  // for EGLConfig

// OpenGL ES 3.2 namespace
import android.opengl.GLES32;

import android.content.Context;  // for Context

// Event related packages
import android.view.GestureDetector.OnDoubleTapListener;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector;
import android.view.MotionEvent;

// Buffer related packages (nio = Native I/O or Non-blocking I/O)
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

// Matrix package
import android.opengl.Matrix;

public class GLESView extends GLSurfaceView implements OnDoubleTapListener, OnGestureListener, GLSurfaceView.Renderer {
    public static final int KVD_ATTRIBUTE_POSITION = 0;
    public static final int KVD_ATTRIBUTE_COLOR = 1;
    public static final int KVD_ATTRIBUTE_NORMAL = 2;
    public static final int KVD_ATTRIBUTE_TEXTURE0 = 3;
    
    private GestureDetector gestureDetector;
    private Context context;

    private int[] vaoSphere = new int[1];
    private int[] vboSpherePositions = new int[1];
    private int[] vboSphereNormals = new int[1];
    private int[] vboSphereElements = new int[1];
    private int shaderProgramObject;

    private int ldUniform;
    private int kdUniform;
    private int lightPositionUniform;
    private int lightEnabledUniform;
    private int modelViewMatrixUniform;
    private int projectionMatrixUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

    private boolean lightSwitch = false;
    private int numSphereVertices;
    private int numSphereElements;
    private float theta;

    public GLESView(Context _context) {
        super(_context);
        
        context = _context;

        // use latest OpenGL ES version 3.2, based on EGL (Embedded Graphics Library) at impl. level (NDK)
        setEGLContextClientVersion(3);
        
        // use this class as the renderer
        setRenderer(this);

        // when view is dirty, invalidate it
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    /* implementing GLSurfaceView.Renderer */
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        System.out.println("KVD: OpenGLES version: " + gl.glGetString(GL10.GL_VERSION));
        System.out.println("KVD: Renderer: " + gl.glGetString(GL10.GL_RENDERER));
        System.out.println("KVD: GLSL version: " + gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION));

        initialize();
    }

    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height) {
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 unused) {
        /*
         * consider as the game loop
         */
        
        update();
        render();
    }

    /* of this view itself */
    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if(!gestureDetector.onTouchEvent(e))
            super.onTouchEvent(e);

        return true;
    }

    /* implementing OnDoubleTapListener */
    @Override
    public boolean onDoubleTap(MotionEvent e) {
        lightSwitch = !(lightSwitch);
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e) {
        /* 
         * onDoubleTapEvent() may or may not carry forward to super,
         * but onDoubleTap() always does; that's the reason we have
         * handled it and not this
         */

        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        /*
         * this trigger shall toggle animation
         */

        return true;
    }

    /* implementing OnGestureListener */
    @Override
    public boolean onDown(MotionEvent e) {
        
        return true;
    }

    @Override
    public boolean onFling(MotionEvent eIn, MotionEvent eOut, float velocityX, float velocityY) {
        
        return true;
    }

    @Override
    public void onLongPress(MotionEvent e) {
        /*
         * this trigger shall toggle textures
         */
    }

    @Override
    public boolean onScroll(MotionEvent eIn, MotionEvent eOut, float distanceX, float distanceY) {
        uninitialize();
        System.exit(0);

        return true;
    }

    @Override
    public void onShowPress(MotionEvent e) {
    
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e) {

        return true;
    }

    /* original methods of this class */
    private void initialize() {
        int[] status = new int[1];
        int[] infoLogLength = new int[1];
        String infoLog = null;

        // vertex shader
        final String vertexShaderSourceCode = String.format(
            "#version 320 es\n" +
            "\n" +
            "in vec4 a_position;\n" +
            "in vec3 a_normal;\n" +
            "\n" +
            "uniform vec4 u_Ld;\n" +
            "uniform vec4 u_Kd;\n" +
            "uniform vec4 u_lightPosition;  // in world space\n" +
            "\n" +
            "uniform int u_lightEnabled;\n" +
            "\n" +
            "uniform mat4 u_modelViewMatrix;\n" +
            "uniform mat4 u_projectionMatrix;\n" +
            "\n" +
            "out vec4 a_color_out;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "vec4 eyeCoordinate = u_modelViewMatrix * a_position;\n" +
                "if(u_lightEnabled > 0)\n" +
                "{\n" +
                    "vec3 lightDirection = normalize(u_lightPosition.xyz - eyeCoordinate.xyz);\n" +
                    "\n" +
                    "mat3 normalMatrix = mat3(transpose(inverse(u_modelViewMatrix)));\n" +
                    "vec3 transformedNormal = normalize(normalMatrix * a_normal);\n" +
                    "\n" +
                    "a_color_out = u_Ld * u_Kd * max(dot(lightDirection, transformedNormal), 0.0);\n" +
                "}\n" +
                "else\n" +
                "{\n" +
                    "a_color_out = vec4(1.0);\n" +
                "}\n" +
                "\n" +
                "gl_Position = u_projectionMatrix * eyeCoordinate;\n" +
            "}\n"
        );

        int vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);
        GLES32.glCompileShader(vertexShaderObject);
        GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, status, 0);
        if(status[0] == GLES32.GL_FALSE) {
            System.out.println("KVD: *** vertex shader compilation errors ***");

            GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0) {
                infoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
                System.out.println("KVD: vertex shader compilation log (" + infoLogLength[0] + " bytes):");
                System.out.println("KVD: " + infoLog);
            }
            else {
                System.out.println("KVD: there is nothing to print");
            }

            uninitialize();
            System.exit(0);
        }

        status[0] = 0;
        infoLogLength[0] = 0;
        infoLog = null;

        // fragment shader
        final String fragmentShaderSourceCode = String.format(
            "#version 320 es\n" +
            "precision highp float;\n" +
            "\n" +
            "in vec4 a_color_out;\n" +
            "\n" +
            "out vec4 FragColor;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "FragColor = a_color_out;\n" +
            "}\n"
        );

        int fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);
        GLES32.glCompileShader(fragmentShaderObject);
        GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, status, 0);
        if(status[0] == GLES32.GL_FALSE) {
            System.out.println("KVD: *** fragment shader compilation errors ***");

            GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0) {
                infoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
                System.out.println("KVD: fragment shader compilation log (" + infoLogLength[0] + " bytes):");
                System.out.println("KVD: " + infoLog);
            }
            else {
                System.out.println("KVD: there is nothing to print");
            }

            uninitialize();
            System.exit(0);
        }
        
        status[0] = 0;
        infoLogLength[0] = 0;
        infoLog = null;

        shaderProgramObject = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);
        GLES32.glBindAttribLocation(shaderProgramObject, GLESView.KVD_ATTRIBUTE_POSITION, "a_position");
        GLES32.glBindAttribLocation(shaderProgramObject, GLESView.KVD_ATTRIBUTE_NORMAL, "a_normal");
        GLES32.glLinkProgram(shaderProgramObject);
        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, status, 0);
        if(status[0] == GLES32.GL_FALSE) {
            System.out.println("KVD: *** there were linking errors ***");

            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0) {
                infoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
                System.out.println("KVD: link time info log (" + infoLogLength[0] + " bytes):");
                System.out.println("KVD : " + infoLog);
            }
            else {
                System.out.println("KVD: there is nothing to print");
            }

            uninitialize();
            System.exit(0);
        }

        // get all uniform locations
        ldUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld");
        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition");
        lightEnabledUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightEnabled");
        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
        modelViewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_modelViewMatrix");

        // sphere vao
        GLES32.glGenVertexArrays(1, vaoSphere, 0);
        GLES32.glBindVertexArray(vaoSphere[0]);
        {
            // generating a sphere model object
            Sphere sphere = new Sphere();

            // data store for the sphere
            float[] sphereVertexPositions = new float[1146];
            float[] sphereVertexNormals = new float[1146];
            float[] sphereVertexTexCoords = new float[764];
            short[] sphereVertexElements = new short[2280];

            // get model data
            sphere.getSphereVertexData(sphereVertexPositions, sphereVertexNormals, sphereVertexTexCoords, sphereVertexElements);
            numSphereVertices = sphere.getNumberOfSphereVertices();
            numSphereElements = sphere.getNumberOfSphereElements();

            // positions vbo
            GLES32.glGenBuffers(1, vboSpherePositions, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboSpherePositions[0]);
            {
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(sphereVertexPositions.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer vertexPositionsBuffer = byteBuffer.asFloatBuffer();
                vertexPositionsBuffer.put(sphereVertexPositions);
                vertexPositionsBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, sphereVertexPositions.length * 4, vertexPositionsBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_POSITION);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

            // normals vbo
            GLES32.glGenBuffers(1, vboSphereNormals, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboSphereNormals[0]);
            {
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(sphereVertexNormals.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer vertexNormalsBuffer = byteBuffer.asFloatBuffer();
                vertexNormalsBuffer.put(sphereVertexNormals);
                vertexNormalsBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, sphereVertexNormals.length * 4, vertexNormalsBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_NORMAL);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

            // texCoords aren't supported in Sphere.java

            // elements vbo
            GLES32.glGenBuffers(1, vboSphereElements, 0);
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
            {
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(sphereVertexElements.length * 2);
                byteBuffer.order(ByteOrder.nativeOrder());

                ShortBuffer vertexElementsBuffer = byteBuffer.asShortBuffer();
                vertexElementsBuffer.put(sphereVertexElements);
                vertexElementsBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER, sphereVertexElements.length * 2, vertexElementsBuffer, GLES32.GL_STATIC_DRAW);
            }
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        GLES32.glBindVertexArray(0);

        // regular OpenGL setup
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // no culling as both sides shall be visible when rotating
        GLES32.glCullFace(GLES32.GL_BACK);
        GLES32.glEnable(GLES32.GL_CULL_FACE);

        // initialize projection matrix
        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);

        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    private void resize(int width, int height) {
        height = (height <= 0 ? 1 : height);

        GLES32.glViewport(0, 0, width, height);

        float aspectRatio = (float)width / (float)height;
        Matrix.perspectiveM(
            perspectiveProjectionMatrix, 0,
            45.0f,
            aspectRatio,
            0.1f,
            100.0f
        );
    }

    private void render() {
        float[] translationMatrix = new float[16];
        float[] rotationMatrix = new float[16];
        float[] modelViewMatrix = new float[16];

        final float[] diffuseLightColor = { 0.8f, 0.8f, 0.8f, 1.0f };
        final float[] diffuseMaterialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        final float[] lightPositionWorldSpace = { 0.0f, 0.0f, 10.0f, 1.0f };

        Matrix.setIdentityM(translationMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);
        Matrix.setIdentityM(modelViewMatrix, 0);

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        GLES32.glUseProgram(shaderProgramObject);
        {
            // transformations for the sphere
            Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -1.5f);
            Matrix.multiplyMM(
                modelViewMatrix, 0,
                translationMatrix, 0,
                modelViewMatrix, 0
            );
            
            // setting matrix uniforms
            GLES32.glUniformMatrix4fv(modelViewMatrixUniform, 1, false, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

            // setting lighting uniforms
            if(lightSwitch) {
                GLES32.glUniform4fv(ldUniform, 1, diffuseLightColor, 0);
                GLES32.glUniform4fv(kdUniform, 1, diffuseMaterialColor, 0);
                GLES32.glUniform4fv(lightPositionUniform, 1, lightPositionWorldSpace, 0);
                GLES32.glUniform1i(lightEnabledUniform, 1);
            }
            else
                GLES32.glUniform1i(lightEnabledUniform, 0);

            // drawing the sphere vertex array
            GLES32.glBindVertexArray(vaoSphere[0]);
            {
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            GLES32.glBindVertexArray(0);
        }
        GLES32.glUseProgram(0);

        requestRender();  // swapping buffers
    }

    private void update() {
        theta += 1.0f;
        if(theta > 360.0f)
            theta -= 360.0f;
    }

    private void uninitialize() {
        for(int i = 0; i >= 0; i--) {
            if(vboSphereElements[i] > 0) {
                GLES32.glDeleteBuffers(1, vboSphereElements, i);
                vboSphereElements[i] = 0;
            }

            if(vboSphereNormals[i] > 0) {
                GLES32.glDeleteBuffers(1, vboSphereNormals, i);
                vboSphereNormals[i] = 0;
            }

            if(vboSpherePositions[i] > 0) {
                GLES32.glDeleteBuffers(1, vboSpherePositions, i);
                vboSpherePositions[i] = 0;
            }

            if(vaoSphere[i] > 0) {
                GLES32.glDeleteVertexArrays(1, vaoSphere, i);
                vaoSphere[i] = 0;
            }
        }

        if(shaderProgramObject > 0) {
            int[] retval = new int[1];

            GLES32.glUseProgram(shaderProgramObject);
            {
                GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_ATTACHED_SHADERS, retval, 0);
                if(retval[0] > 0) {
                    int numAttachedShaders = retval[0];
                    int[] shaderObjects = new int[numAttachedShaders];
                    
                    GLES32.glGetAttachedShaders(shaderProgramObject, numAttachedShaders, retval, 0, shaderObjects, 0);
                    for(int i = 0; i < numAttachedShaders; i++) {
                        GLES32.glDetachShader(shaderProgramObject, shaderObjects[i]);
                        GLES32.glDeleteShader(shaderObjects[i]);
                        shaderObjects[i] = 0;
                    }

                    System.out.println("KVD: detached and deleted " + numAttachedShaders + " shader objects");
                }
            }
            GLES32.glUseProgram(0);

            GLES32.glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;
            System.out.println("KVD: deleted shader program object");
        }
    }
}
