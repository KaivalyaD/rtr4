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

    private int shaderProgramObject;

    private int[] vao = new int[1];
    private int[] vboPositions = new int[1];
    private int[] vboNormals = new int[1];
    private int viewPositionUniform;

    private GLLight[] light = new GLLight[2];
    private boolean lightSwitch = false;

    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int materialShininessUniform;

    private int modelViewMatrixUniform;
    private int projectionMatrixUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

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
            "// light properties\n" +
            "uniform vec4 u_lightPosition[2];  // in world space\n" +
            "\n" +
            "// viewer properties\n" +
            "uniform vec4 u_viewPosition;  // in world space\n" +
            "\n" +
            "/*\n" +
            " * FS has NO default precision set for ANY of the types to impose less \n" +
            " * restrictions on ES vendors. It hence MUST be explicitly specified whenever\n" +
            " * uniforms are be accessed in the fragment and another shader.\n" +
            " * The default for int in VS is mediump.\n" +
            " */\n" +
            "uniform lowp int u_lightEnabled;\n" +
            "\n" +
            "uniform mat4 u_modelViewMatrix;\n" +
            "uniform mat4 u_projectionMatrix;\n" +
            "\n" +
            "out vec3 lightDirection[2];\n" +
            "out vec3 transformedNormal;\n" +
            "out vec3 viewerVector;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "vec4 eyeCoordinate = u_modelViewMatrix * a_position;\n" +
                "if(u_lightEnabled > 0)\n" +
                "{\n" +
                    "lightDirection[0] = normalize(u_lightPosition[0].xyz - eyeCoordinate.xyz);\n" +
                    "lightDirection[1] = normalize(u_lightPosition[1].xyz - eyeCoordinate.xyz);\n" +
                    "\n" +
                    "mat3 normalMatrix = mat3(transpose(inverse(u_modelViewMatrix)));\n" +
                    "transformedNormal = normalize(normalMatrix * a_normal);\n" +
                    "\n" +
                    "viewerVector = normalize(u_viewPosition.xyz - eyeCoordinate.xyz);\n" +
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
            "in vec3 lightDirection[2];\n" +
            "in vec3 transformedNormal;\n" +
            "in vec3 viewerVector;\n" +
            "\n" +
            "// light properties\n" +
            "uniform vec4 u_La[2];\n" +
            "uniform vec4 u_Ld[2];\n" +
            "uniform vec4 u_Ls[2];\n" +
            "\n" +
            "// material properties\n" +
            "uniform vec4 u_Ka;\n" +
            "uniform vec4 u_Kd;\n" +
            "uniform vec4 u_Ks;\n" +
            "uniform float u_materialShininess;\n" +
            "\n" +
            "/*\n" +
            " * FS has NO default precision set for ANY of the types to impose less \n" +
            " * restrictions on ES vendors. It hence MUST be explicitly specified whenever\n" +
            " * uniforms are be accessed in the fragment and another shader.\n" +
            " * The default for int in VS is mediump.\n" +
            " */\n" +
            "uniform lowp int u_lightEnabled;\n" +
            "\n" +
            "out vec4 FragColor;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "if(u_lightEnabled > 0)\n" +
                "{\n" +
                    "vec3 normLightDirection[2];\n" +
                    "normLightDirection[0] = normalize(lightDirection[0]);\n" +
                    "normLightDirection[1] = normalize(lightDirection[1]);\n" +
                    "vec3 normal = normalize(transformedNormal);\n" +
                    "vec3 normViewerVector = normalize(viewerVector);\n" +
                    "\n" +
                    "vec3 reflectedRay[2];\n" +
                    "reflectedRay[0] = reflect(-normLightDirection[0], normal);\n" +
                    "reflectedRay[1] = reflect(-normLightDirection[1], normal);\n" +
                    "\n" +
                    "vec4 ambient[2], diffuse[2], specular[2];\n" +
                    "\n" +
                    "ambient[0] = u_La[0] * u_Ka;\n" +
                    "diffuse[0] = u_Ld[0] * u_Kd * max(dot(normLightDirection[0], normal), 0.0);\n" +
                    "specular[0] = u_Ls[0] * u_Ks * pow(max(dot(reflectedRay[0], normViewerVector), 0.0), u_materialShininess);\n" +
                    "\n" +
                    "ambient[1] = u_La[1] * u_Ka;\n" +
                    "diffuse[1] = u_Ld[1] * u_Kd * max(dot(normLightDirection[1], normal), 0.0);\n" +
                    "specular[1] = u_Ls[1] * u_Ks * pow(max(dot(reflectedRay[1], normViewerVector), 0.0), u_materialShininess);\n" +
                    "\n" +
                    "FragColor = (ambient[0] + diffuse[0] + specular[0]) + (ambient[1] + diffuse[1] + specular[1]);\n" +
                "}\n" +
                "else\n" +
                "{\n" +
                    "FragColor = vec4(1.0);\n" +
                "}\n" +
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
                System.out.println("KVD: " + infoLog);
            }
            else {
                System.out.println("KVD: there is nothing to print");
            }

            uninitialize();
            System.exit(0);
        }

        // allocating memory for lights
        light[0] = new GLLight();
        light[1] = new GLLight();

        // get all uniform locations
        light[0].setAmbientColorUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_La[0]"));
        light[0].setDiffuseColorUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld[0]"));
        light[0].setSpecularColorUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls[0]"));
        light[0].setPositionUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition[0]"));
        light[0].setSwitchUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_lightEnabled"));

        light[1].setAmbientColorUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_La[1]"));
        light[1].setDiffuseColorUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld[1]"));
        light[1].setSpecularColorUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls[1]"));
        light[1].setPositionUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition[1]"));
        light[1].setSwitchUniformLocation(GLES32.glGetUniformLocation(shaderProgramObject, "u_lightEnabled"));

        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ka");
        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ks");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialShininess");

        viewPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_viewPosition");

        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
        modelViewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_modelViewMatrix");

        // pyramid vao
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);
        {
            // positions vbo
            GLES32.glGenBuffers(1, vboPositions, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositions[0]);
            {
                final float[] vertexPositions = {
                    // front face
                    0.0f,  1.0f, 0.0f,
                   -1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,

                    // right face
                    0.0f,  1.0f,  0.0f,
                    1.0f, -1.0f,  1.0f,
                    1.0f, -1.0f, -1.0f,

                    // rear face
                    0.0f,  1.0f,  0.0f,
                    1.0f, -1.0f, -1.0f,
                   -1.0f, -1.0f, -1.0f,

                    // left face
                    0.0f,  1.0f,  0.0f,
                   -1.0f, -1.0f, -1.0f,
                   -1.0f, -1.0f,  1.0f
                };
                
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(vertexPositions.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer vertexPositionsBuffer = byteBuffer.asFloatBuffer();
                vertexPositionsBuffer.put(vertexPositions);
                vertexPositionsBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertexPositions.length * 4, vertexPositionsBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_POSITION);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

            // normals vbo
            GLES32.glGenBuffers(1, vboNormals, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboNormals[0]);
            {
                final float[] vertexNormals = {
                    0.0f, 0.447214f, 0.894427f, // front-top
                    0.0f, 0.447214f, 0.894427f, // front-left
                    0.0f, 0.447214f, 0.894427f, // front-right

                    0.894427f, 0.447214f, 0.0f, // right-top
                    0.894427f, 0.447214f, 0.0f, // right-left
                    0.894427f, 0.447214f, 0.0f, // right-right

                    0.0f, 0.447214f, -0.894427f, // rear-top
                    0.0f, 0.447214f, -0.894427f, // rear-left
                    0.0f, 0.447214f, -0.894427f, // rear-right

                   -0.894427f, 0.447214f, 0.0f, // left-top
                   -0.894427f, 0.447214f, 0.0f, // left-left
                   -0.894427f, 0.447214f, 0.0f  // left-right
                };
                
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(vertexNormals.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer vertexNormalsBuffer = byteBuffer.asFloatBuffer();
                vertexNormalsBuffer.put(vertexNormals);
                vertexNormalsBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertexNormals.length * 4, vertexNormalsBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_NORMAL);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
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

        // setting light properties
        light[0].setAmbientColor(1.0f, 1.0f, 1.0f, 1.0f);
        light[0].setDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
        light[0].setSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
        light[0].setPosition(-1.0f, 0.0f, 0.0f, 1.0f);

        light[1].setAmbientColor(1.0f, 1.0f, 1.0f, 1.0f);
        light[1].setDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);
        light[1].setSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
        light[1].setPosition(1.0f, 0.0f, 0.0f, 1.0f);

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

        final float[] ambientMaterialColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        final float[] diffuseMaterialColor = { 0.5f, 0.2f, 0.7f, 1.0f };
        final float[] specularMaterialColor = { 0.7f, 0.7f, 0.7f, 1.0f };
        final float materialShininess = 128.0f;

        final float[] viewPositionWorldSpace = { 0.0f, 0.0f, 5.0f, 1.0f };

        Matrix.setIdentityM(translationMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);
        Matrix.setIdentityM(modelViewMatrix, 0);

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        GLES32.glUseProgram(shaderProgramObject);
        {
            // transformations for the pyramid
            Matrix.rotateM(rotationMatrix, 0, theta, 0.0f, 1.0f, 0.0f);
            Matrix.multiplyMM(
                modelViewMatrix, 0,
                rotationMatrix, 0,
                modelViewMatrix, 0
            );

            Matrix.translateM(translationMatrix, 0, viewPositionWorldSpace[0], viewPositionWorldSpace[1], -viewPositionWorldSpace[2]);
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
                GLES32.glUniform4fv(light[0].getAmbientColorUniformLocation(), 1, light[0].getAmbientColor(), 0);
                GLES32.glUniform4fv(light[0].getDiffuseColorUniformLocation(), 1, light[0].getDiffuseColor(), 0);
                GLES32.glUniform4fv(light[0].getSpecularColorUniformLocation(), 1, light[0].getSpecularColor(), 0);
                GLES32.glUniform4fv(light[0].getPositionUniformLocation(), 1, light[0].getPosition(), 0);

                GLES32.glUniform4fv(light[1].getAmbientColorUniformLocation(), 1, light[1].getAmbientColor(), 0);
                GLES32.glUniform4fv(light[1].getDiffuseColorUniformLocation(), 1, light[1].getDiffuseColor(), 0);
                GLES32.glUniform4fv(light[1].getSpecularColorUniformLocation(), 1, light[1].getSpecularColor(), 0);
                GLES32.glUniform4fv(light[1].getPositionUniformLocation(), 1, light[1].getPosition(), 0);

                GLES32.glUniform4fv(kaUniform, 1, ambientMaterialColor, 0);
                GLES32.glUniform4fv(kdUniform, 1, diffuseMaterialColor, 0);
                GLES32.glUniform4fv(ksUniform, 1, specularMaterialColor, 0);
                GLES32.glUniform1f(materialShininessUniform, materialShininess);

                GLES32.glUniform4fv(viewPositionUniform, 1, viewPositionWorldSpace, 0);
                GLES32.glUniform1i(light[0].getSwitchUniformLocation(), 1);
                GLES32.glUniform1i(light[1].getSwitchUniformLocation(), 1);
            }
            else {
                GLES32.glUniform1i(light[0].getSwitchUniformLocation(), 0);
                GLES32.glUniform1i(light[1].getSwitchUniformLocation(), 0);
            }

            // drawing the pyramid vertex array
            GLES32.glBindVertexArray(vao[0]);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
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
            if(vboNormals[i] > 0) {
                GLES32.glDeleteBuffers(1, vboNormals, i);
                vboNormals[i] = 0;
            }

            if(vboPositions[i] > 0) {
                GLES32.glDeleteBuffers(1, vboPositions, i);
                vboPositions[i] = 0;
            }

            if(vao[i] > 0) {
                GLES32.glDeleteVertexArrays(1, vao, i);
                vao[i] = 0;
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
