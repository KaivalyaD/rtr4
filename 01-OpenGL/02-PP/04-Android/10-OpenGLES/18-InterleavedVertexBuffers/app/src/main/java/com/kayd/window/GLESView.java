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

// Packages required to load texture images
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.opengl.GLUtils;

public class GLESView extends GLSurfaceView implements OnDoubleTapListener, OnGestureListener, GLSurfaceView.Renderer {
    public static final int KVD_ATTRIBUTE_POSITION = 0;
    public static final int KVD_ATTRIBUTE_COLOR = 1;
    public static final int KVD_ATTRIBUTE_NORMAL = 2;
    public static final int KVD_ATTRIBUTE_TEXTURE0 = 3;
    
    private GestureDetector gestureDetector;
    private Context context;

    private int[] vao = new int[1];
    private int[] vbo = new int[1];
    private int shaderProgramObject;

    private int laUniform;
    private int ldUniform;
    private int lsUniform;
    private int lightPositionUniform;

    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int materialShininessUniform;

    private int viewPositionUniform;
    private int lightEnabledUniform;

    private int[] textureMarble = new int[1];
    private int textureSamplerUniform;

    private int modelViewMatrixUniform;
    private int projectionMatrixUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

    private boolean lightSwitch = false;
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
            "in vec4 a_color;\n" +
            "in vec3 a_normal;\n" +
            "in vec2 a_texCoord;\n" +
            "\n" +
            "// light properties\n" +
            "uniform vec4 u_lightPosition;  // in world space\n" +
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
            "out vec3 lightDirection;\n" +
            "out vec3 transformedNormal;\n" +
            "out vec3 viewerVector;\n" +
            "out vec4 color;\n" +
            "out vec2 texCoord;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "vec4 eyeCoordinate = u_modelViewMatrix * a_position;\n" +
                "if(u_lightEnabled > 0)\n" +
                "{\n" +
                    "lightDirection = normalize(u_lightPosition.xyz - eyeCoordinate.xyz);\n" +
                    "\n" +
                    "mat3 normalMatrix = mat3(transpose(inverse(u_modelViewMatrix)));\n" +
                    "transformedNormal = normalize(normalMatrix * a_normal);\n" +
                    "\n" +
                    "viewerVector = normalize(u_viewPosition.xyz - eyeCoordinate.xyz);\n" +
                "}\n" +
                "\n" +
                "gl_Position = u_projectionMatrix * eyeCoordinate;\n" +
                "color = a_color;\n" +
                "texCoord = a_texCoord;\n" +
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
            "in vec3 lightDirection;\n" +
            "in vec3 transformedNormal;\n" +
            "in vec3 viewerVector;\n" +
            "in vec4 color;\n" +
            "in vec2 texCoord;\n" +
            "\n" +
            "// light properties\n" +
            "uniform vec4 u_La;\n" +
            "uniform vec4 u_Ld;\n" +
            "uniform vec4 u_Ls;\n" +
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
            "// texture uniforms\n" +
            "uniform highp sampler2D u_textureSampler;\n" +
            "\n" +
            "out vec4 FragColor;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "if(u_lightEnabled > 0)\n" +
                "{\n" +
                    "vec3 normLightDirection = normalize(lightDirection);\n" +
                    "vec3 normal = normalize(transformedNormal);\n" +
                    "vec3 normViewerVector = normalize(viewerVector);\n" +
                    "\n" +
                    "vec3 reflectedRay = reflect(-normLightDirection, normal);\n" +
                    "\n" +
                    "vec4 ambient = u_La * u_Ka;\n" +
                    "vec4 diffuse = u_Ld * u_Kd * max(dot(normLightDirection, normal), 0.0);\n" +
                    "vec4 specular = u_Ls * u_Ks * pow(max(dot(reflectedRay, normViewerVector), 0.0), u_materialShininess);\n" +
                    "\n" +
                    "FragColor = color * texture(u_textureSampler, texCoord) * (ambient + diffuse + specular);\n" +
                "}\n" +
                "else\n" +
                "{\n" +
                    "FragColor = color * texture(u_textureSampler, texCoord);\n" +
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
        GLES32.glBindAttribLocation(shaderProgramObject, GLESView.KVD_ATTRIBUTE_COLOR, "a_color");
        GLES32.glBindAttribLocation(shaderProgramObject, GLESView.KVD_ATTRIBUTE_NORMAL, "a_normal");
        GLES32.glBindAttribLocation(shaderProgramObject, GLESView.KVD_ATTRIBUTE_TEXTURE0, "a_texCoord");
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

        // get all uniform locations
        laUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_La");
        ldUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld");
        lsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition");

        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ka");
        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ks");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialShininess");

        viewPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_viewPosition");
        lightEnabledUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightEnabled");

        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
        modelViewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_modelViewMatrix");

        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_textureSampler");

        // vao
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);
        {
            // vertex data
            final float cubeVertexDataInterleaved[] = {
                // Positions             // Colors               // Normals                    // TexCoords
                // Top face              // Top face             // Top face                   // Top face
                1.0f,  1.0f, -1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           1.0f, 1.0f,
               -1.0f,  1.0f, -1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           0.0f, 1.0f,
               -1.0f,  1.0f,  1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           0.0f, 0.0f,
                1.0f,  1.0f,  1.0f,      0.0f, 0.0f, 1.0f,       0.0f,  1.0f,  0.0f,           1.0f, 0.0f,

                // Bottom face           // Bottom face          // Bottom face                // Bottom face
                1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           1.0f, 1.0f,
               -1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           0.0f, 1.0f,
               -1.0f, -1.0f,  1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           0.0f, 0.0f,
                1.0f, -1.0f,  1.0f,      0.0f, 1.0f, 0.0f,       0.0f, -1.0f,  0.0f,           1.0f, 0.0f,

                // Front face            // Front face           // Front face                 // Front face
                1.0f,  1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           1.0f, 1.0f,
               -1.0f,  1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           0.0f, 1.0f,
               -1.0f, -1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           0.0f, 0.0f,
                1.0f, -1.0f,  1.0f,      1.0f, 0.0f, 0.0f,       0.0f,  0.0f,  1.0f,           1.0f, 0.0f,

                // Back face             // Back face            // Back face                  // Back face
                1.0f,  1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           1.0f, 1.0f,
               -1.0f,  1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           0.0f, 1.0f,
               -1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           0.0f, 0.0f,
                1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 1.0f,       0.0f,  0.0f, -1.0f,           1.0f, 0.0f,

                // Right face            // Right face           // Right face                 // Right face
                1.0f,  1.0f, -1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           1.0f, 1.0f,
                1.0f,  1.0f,  1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           0.0f, 1.0f,
                1.0f, -1.0f,  1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           0.0f, 0.0f,
                1.0f, -1.0f, -1.0f,      1.0f, 0.0f, 1.0f,       1.0f,  0.0f,  0.0f,           1.0f, 0.0f,

                // Left face             // Left face             // Left face                 // Left face
               -1.0f,  1.0f,  1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           1.0f, 1.0f,
               -1.0f,  1.0f, -1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           0.0f, 1.0f,
               -1.0f, -1.0f, -1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           0.0f, 0.0f,
               -1.0f, -1.0f,  1.0f,      1.0f, 1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,           1.0f, 0.0f
            };

            GLES32.glGenBuffers(1, vbo, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);
            {
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(cubeVertexDataInterleaved.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer cubeVertexDataBuffer = byteBuffer.asFloatBuffer();
                cubeVertexDataBuffer.put(cubeVertexDataInterleaved);
                cubeVertexDataBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeVertexDataInterleaved.length * 4, cubeVertexDataBuffer, GLES32.GL_STATIC_DRAW);
                
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 11 * 4, 0 * 4);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 11 * 4, 3 * 4);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 11 * 4, 6 * 4);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_TEXTURE0, 2, GLES32.GL_FLOAT, false, 11 * 4, 9 * 4);
                
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_POSITION);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_COLOR);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_NORMAL);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_TEXTURE0);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        }
        GLES32.glBindVertexArray(0);

        // load required textures
        textureMarble[0] = loadGLTexture(R.raw.marble);
        if(textureMarble[0] < 0)
        {
            System.out.println("KVD: loadGLTexture(): failed for textureMarble");
            System.exit(0);
        }
        else
            System.out.println("KVD: loadGLTexture(): loaded textureMarble");

        // regular OpenGL setup
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        GLES32.glDisable(GLES32.GL_CULL_FACE);

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
        float[] scaleMatrix = new float[16];
        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];

        final float[] ambientLightColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        final float[] diffuseLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        final float[] specularLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        final float[] lightPositionWorldSpace = { 0.0f, 0.0f, 10.0f, 1.0f };

        final float[] ambientMaterialColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        final float[] diffuseMaterialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        final float[] specularMaterialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        final float materialShininess = 128.0f;

        final float[] viewPositionWorldSpace = { 0.0f, 0.0f, 0.0f, 1.0f };

        Matrix.setIdentityM(translationMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);
        Matrix.setIdentityM(scaleMatrix, 0);
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        GLES32.glUseProgram(shaderProgramObject);
        {
            // transformations
            Matrix.multiplyMM(
                modelViewMatrix, 0,
                scaleMatrix, 0,
                modelViewMatrix, 0
            );

            Matrix.rotateM(rotationMatrix, 0, theta, 1.0f, 1.0f, 1.0f);
            Matrix.multiplyMM(
                modelViewMatrix, 0,
                rotationMatrix, 0,
                modelViewMatrix, 0
            );

            Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -5.0f);
            Matrix.multiplyMM(
                modelViewMatrix, 0,
                translationMatrix, 0,
                modelViewMatrix, 0
            );

            Matrix.multiplyMM(
                modelViewProjectionMatrix, 0,
                perspectiveProjectionMatrix, 0,
                modelViewMatrix, 0
            );
            
            // setting matrix uniforms
            GLES32.glUniformMatrix4fv(modelViewMatrixUniform, 1, false, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

            // setting lighting uniforms
            if(lightSwitch) {
                GLES32.glUniform4fv(laUniform, 1, ambientLightColor, 0);
                GLES32.glUniform4fv(ldUniform, 1, diffuseLightColor, 0);
                GLES32.glUniform4fv(lsUniform, 1, specularLightColor, 0);
                GLES32.glUniform4fv(lightPositionUniform, 1, lightPositionWorldSpace, 0);

                GLES32.glUniform4fv(kaUniform, 1, ambientMaterialColor, 0);
                GLES32.glUniform4fv(kdUniform, 1, diffuseMaterialColor, 0);
                GLES32.glUniform4fv(ksUniform, 1, specularMaterialColor, 0);
                GLES32.glUniform1f(materialShininessUniform, materialShininess);

                GLES32.glUniform4fv(viewPositionUniform, 1, viewPositionWorldSpace, 0);
                GLES32.glUniform1i(lightEnabledUniform, 1);
            }
            else
                GLES32.glUniform1i(lightEnabledUniform, 0);

            // setup textures
            GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
            GLES32.glUniform1i(textureSamplerUniform, 0);
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, textureMarble[0]);

            // drawing the sphere vertex array
            GLES32.glBindVertexArray(vao[0]);
            {
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
                GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
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
            if(vbo[i] > 0) {
                GLES32.glDeleteBuffers(1, vbo, i);
                vbo[i] = 0;
            }

            if(vao[i] > 0) {
                GLES32.glDeleteVertexArrays(1, vao, i);
                vao[i] = 0;
            }
        }

        if(textureMarble[0] > 0) {
            GLES32.glDeleteTextures(1, textureMarble, 0);
            textureMarble[0] = 0;
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

    private int loadGLTexture(int resourceId) {
        // prevent Android from downscaling the image
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;

        // load the image
        Bitmap bitmap = BitmapFactory.decodeResource(
            context.getResources(),
            resourceId,
            options
        );
        if(bitmap == null)
            return -1;

        // feed the image to GLES and get the texture name
        int[] textureName = new int[1];

        GLES32.glPixelStorei(GLES32.GL_UNPACK_ALIGNMENT, 1);
        GLES32.glGenTextures(1, textureName, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, textureName[0]);
        {
            GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
            GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR_MIPMAP_LINEAR);
            GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_S, GLES32.GL_REPEAT);
            GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_T, GLES32.GL_REPEAT);
            GLUtils.texImage2D(
                GLES32.GL_TEXTURE_2D,
                0,
                bitmap,
                0
            );
            GLES32.glGenerateMipmap(GLES32.GL_TEXTURE_2D);
        }
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);

        return textureName[0];
    }
}
