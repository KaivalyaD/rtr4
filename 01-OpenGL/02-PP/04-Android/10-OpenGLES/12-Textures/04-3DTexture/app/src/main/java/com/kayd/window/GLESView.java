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

    private int[] vao = new int[2];
    private int[] vboPositions = new int[2];
    private int[] vboTexCoords = new int[2];
    private int mvpMatrixUniform;
    private int textureSamplerUniform;
    private int shaderProgramObject;

    private int[] textureStone = new int[1];
    private int[] textureKundali = new int[1];

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
        /*
         * this trigger shall toggle lighting
         */

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
            "in vec2 a_texCoord;\n" +
            "\n" +
            "uniform mat4 u_mvpMatrix;\n" +
            "\n" +
            "out vec2 a_texCoord_out;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "gl_Position = u_mvpMatrix * a_position;\n" +
                "a_texCoord_out = a_texCoord;\n" +
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
                System.out.println(infoLog);
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
            "in vec2 a_texCoord_out;\n" +
            "\n" +
            "uniform highp sampler2D u_textureSampler;\n" +
            "\n" +
            "out vec4 FragColor;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "FragColor = texture(u_textureSampler, a_texCoord_out);\n" +
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
                System.out.println(infoLog);
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
        GLES32.glBindAttribLocation(shaderProgramObject, GLESView.KVD_ATTRIBUTE_TEXTURE0, "a_texCoord");
        GLES32.glLinkProgram(shaderProgramObject);
        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, status, 0);
        if(status[0] == GLES32.GL_FALSE) {
            System.out.println("KVD: *** there were linking errors ***");

            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0) {
                infoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
                System.out.println("KVD: link time info log (" + infoLogLength[0] + " bytes):");
                System.out.println(infoLog);
            }
            else {
                System.out.println("KVD: there is nothing to print");
            }

            uninitialize();
            System.exit(0);
        }

        // get all uniform locations
        mvpMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_textureSampler");

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

            // texCoords vbo
            GLES32.glGenBuffers(1, vboTexCoords, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboTexCoords[0]);
            {
                // tex coords for the pyramid
                final float[] vertexTexCoords = {
                    // front
                    1.0f, 1.0f,
                    0.0f, 0.0f,
                    2.0f, 0.0f,

                    // right
                    1.0f, 1.0f,
                    0.0f, 0.0f,
                    2.0f, 0.0f,

                    // rear
                    1.0f, 1.0f,
                    2.0f, 0.0f,
                    0.0f, 0.0f,

                    // left
                    1.0f, 1.0f,
                    2.0f, 0.0f,
                    0.0f, 0.0f,
                };

                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(vertexTexCoords.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer vertexTexCoordsBuffer = byteBuffer.asFloatBuffer();
                vertexTexCoordsBuffer.put(vertexTexCoords);
                vertexTexCoordsBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertexTexCoords.length * 4, vertexTexCoordsBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_TEXTURE0, 2, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_TEXTURE0);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        }
        GLES32.glBindVertexArray(0);

        // cube vao
        GLES32.glGenVertexArrays(1, vao, 1);
        GLES32.glBindVertexArray(vao[1]);
        {
            // positions vbo
            GLES32.glGenBuffers(1, vboPositions, 1);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboPositions[1]);
            {
                final float[] vertexPositions = {
                    // front face
                    1.0f,  1.0f,  1.0f,
                   -1.0f,  1.0f,  1.0f,
                   -1.0f, -1.0f,  1.0f,
                    1.0f, -1.0f,  1.0f,

                    // right face
                    1.0f,  1.0f, -1.0f,
                    1.0f,  1.0f,  1.0f,
                    1.0f, -1.0f,  1.0f,
                    1.0f, -1.0f, -1.0f,

                    // rear face
                   -1.0f,  1.0f, -1.0f,
                    1.0f,  1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,
                   -1.0f, -1.0f, -1.0f,

                    // left face
                   -1.0f,  1.0f,  1.0f,
                   -1.0f,  1.0f, -1.0f,
                   -1.0f, -1.0f, -1.0f,
                   -1.0f, -1.0f,  1.0f,

                    // top face
                    1.0f,  1.0f, -1.0f,
                   -1.0f,  1.0f, -1.0f,
                   -1.0f,  1.0f,  1.0f,
                    1.0f,  1.0f,  1.0f,

                    // bottom face
                    1.0f, -1.0f,  1.0f,
                   -1.0f, -1.0f,  1.0f,
                   -1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f
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

            // texCoords vbo
            GLES32.glGenBuffers(1, vboTexCoords, 1);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboTexCoords[1]);
            {
                // tex coords for the cube
                final float[] vertexTexCoords = {
                    // front
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,

                    // right
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,

                    // rear
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f,
                    0.0f, 0.0f,

                    // left
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f,
                    0.0f, 0.0f,

                    // top
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    // bottom
                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f
                };

                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(vertexTexCoords.length * 4);
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer vertexTexCoordsBuffer = byteBuffer.asFloatBuffer();
                vertexTexCoordsBuffer.put(vertexTexCoords);
                vertexTexCoordsBuffer.position(0);

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertexTexCoords.length * 4, vertexTexCoordsBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_TEXTURE0, 2, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_TEXTURE0);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        }
        GLES32.glBindVertexArray(0);

        // loading the needed textures
        textureStone[0] = loadGLTexture(R.raw.stone);
        if(textureStone[0] < 0)
        {
            System.out.println("KVD: loadGLTexture(): failed for textureStone");
            System.exit(0);
        }
        else
            System.out.println("KVD: loadGLTexture(): loaded textureStone");

        textureKundali[0] = loadGLTexture(R.raw.vijay_kundali);
        if(textureKundali[0] < 0)
        {
            System.out.println("KVD: loadGLTexture(): failed for textureKundali");
            System.exit(0);
        }
        else
            System.out.println("KVD: loadGLTexture(): loaded textureKundali");

        // regular OpenGL setup
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // enable culling for better es performance
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
        float[] scaleMatrix = new float[16];
        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];

        Matrix.setIdentityM(translationMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);
        Matrix.setIdentityM(scaleMatrix, 0);
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);

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

            Matrix.translateM(translationMatrix, 0, -1.2f, 0.0f, -4.0f);
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
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            // setting texture uniforms
            GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
            GLES32.glUniform1i(textureSamplerUniform, 0);

            // texture setup for the pyramid
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, textureStone[0]);
            {
                // drawing the pyramid vertex array
                GLES32.glBindVertexArray(vao[0]);
                    GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
                GLES32.glBindVertexArray(0);
            }
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);

            // transformations for the cube
            Matrix.setIdentityM(rotationMatrix, 0);
            Matrix.setIdentityM(translationMatrix, 0);
            Matrix.setIdentityM(modelViewMatrix, 0);

            Matrix.scaleM(scaleMatrix, 0, 0.8f, 0.8f, 0.8f);
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

            Matrix.translateM(translationMatrix, 0, 1.2f, 0.0f, -4.0f);
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
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            // texture setup for the cube
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, textureKundali[0]);
            {
                // drawing the cube vertex array
                GLES32.glBindVertexArray(vao[1]);
                    GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
                    GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
                    GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
                    GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
                    GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
                    GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
                GLES32.glBindVertexArray(0);
            }
            GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
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
        if(textureStone[0] > 0) {
            GLES32.glDeleteTextures(1, textureStone, 0);
            textureStone[0] = 0;
        }

        if(textureKundali[0] > 0){
            GLES32.glDeleteTextures(1, textureKundali, 0);
            textureKundali[0] = 0;
        }
        
        for(int i = 1; i >= 0; i--) {
            if(vboTexCoords[i] > 0) {
                GLES32.glDeleteBuffers(1, vboTexCoords, i);
                vboTexCoords[i] = 0;
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
