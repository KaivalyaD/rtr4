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

public class GLESView extends GLSurfaceView implements OnDoubleTapListener, OnGestureListener, GLSurfaceView.Renderer {
    public static final int KVD_ATTRIBUTE_POSITION = 0;
    public static final int KVD_ATTRIBUTE_COLOR = 1;
    public static final int KVD_ATTRIBUTE_NORMAL = 2;
    public static final int KVD_ATTRIBUTE_TEXTURE0 = 3;
    
    private GestureDetector gestureDetector;
    private Context context;

    private int[] vaoLine = new int[1];
    private int[] vboLine = new int[1];
    private int[] vaoTriangle = new int[1];
    private int[] vboTriangle = new int[1];
    private int[] vaoSquare = new int[1];
    private int[] vboSquare = new int[1];
    private int[] vaoCircle = new int[1];
    private int[] vboCircle = new int[1];

    private int mvpMatrixUniform;
    private int shaderProgramObject;

    // controls
    boolean showTriangle = true;
    boolean showSquare = true;
    boolean showCircle = true;

    private float[] perspectiveProjectionMatrix = new float[16];

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
        showSquare = !showSquare;
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
        showTriangle = !showTriangle;
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
        showCircle = !showCircle;
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
            "\n" +
            "uniform mat4 u_mvpMatrix;\n" +
            "\n" +
            "out vec4 a_color_out;\n" +
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "gl_Position = u_mvpMatrix * a_position;\n" +
                "a_color_out = a_color;\n" +
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
        GLES32.glBindAttribLocation(shaderProgramObject, GLESView.KVD_ATTRIBUTE_COLOR, "a_color");
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

        // axes
        GLES32.glGenVertexArrays(1, vaoLine, 0);
        GLES32.glBindVertexArray(vaoLine[0]);
        {
            GLES32.glGenBuffers(1, vboLine, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboLine[0]);
            {
                final float[] vertexPositions = {
                    1.0f, 0.0f, 0.0f,
                   -1.0f, 0.0f, 0.0f,
                };
                
                /* create a NDK-usable (a.k.a. Direct) buffer (4 = size of float in bytes) */
                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(vertexPositions.length * 4);

                /* for platform independence, decide endianness at runtime */
                byteBuffer.order(ByteOrder.nativeOrder());

                FloatBuffer vertexPositionsBuffer = byteBuffer.asFloatBuffer();
                vertexPositionsBuffer.put(vertexPositions);
                vertexPositionsBuffer.position(0);  // start from 0th offset in array vertexPositions

                GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertexPositions.length * 4, vertexPositionsBuffer, GLES32.GL_STATIC_DRAW);
                GLES32.glVertexAttribPointer(GLESView.KVD_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
                GLES32.glEnableVertexAttribArray(GLESView.KVD_ATTRIBUTE_POSITION);
            }
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);            
        }
        GLES32.glBindVertexArray(0);

        // triangle
        GLES32.glGenVertexArrays(1, vaoTriangle, 0);
        GLES32.glBindVertexArray(vaoTriangle[0]);
        {
            GLES32.glGenBuffers(1, vboTriangle, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboTriangle[0]);
            {
                final float vertexPositions[] = {
                    (float)Math.sin(Math.toRadians(000.0)), (float)Math.cos(Math.toRadians(000.0)), 0.0f,
                    (float)Math.sin(Math.toRadians(120.0)), (float)Math.cos(Math.toRadians(120.0)), 0.0f,
                    (float)Math.sin(Math.toRadians(240.0)), (float)Math.cos(Math.toRadians(240.0)), 0.0f,
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
        }
        GLES32.glBindVertexArray(0);

        // square
        GLES32.glGenVertexArrays(1, vaoSquare, 0);
        GLES32.glBindVertexArray(vaoSquare[0]);
        {
            GLES32.glGenBuffers(1, vboSquare, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboSquare[0]);
            {
                final float vertexPositions[] = {
                    1.0f,  1.0f, 0.0f,
                   -1.0f,  1.0f, 0.0f,
                   -1.0f, -1.0f, 0.0f,
                    1.0f, -1.0f, 0.0f
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
        }
        GLES32.glBindVertexArray(0);

        // circle
        GLES32.glGenVertexArrays(1, vaoCircle, 0);
        GLES32.glBindVertexArray(vaoCircle[0]);
        {
            GLES32.glGenBuffers(1, vboCircle, 0);
            GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCircle[0]);
            {
                float vertexPositions[] = new float[100 * 3];
                for(int i = 0; i < vertexPositions.length; i += 3) {
                    double theta = 2.0f * Math.PI * (float)i / (float)(vertexPositions.length / 3);
                    
                    vertexPositions[i + 0] = (float)Math.cos(theta);
                    vertexPositions[i + 1] = (float)Math.sin(theta);
                    vertexPositions[i + 2] = 0.0f;
                }
                
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
        }
        GLES32.glBindVertexArray(0);

        // regular OpenGL setup
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // enable culling for better performance on es
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
        float[] modelMatrix = new float[16];
        float[] viewMatrix = new float[16];
        float[] modelViewMatrix = new float[16];
        float[] viewProjectionMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];

        float x, y;

        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(viewProjectionMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        GLES32.glUseProgram(shaderProgramObject);
        {
            // view transformations
            Matrix.translateM(viewMatrix, 0, 0.0f, 0.0f, -2.5f);
            Matrix.multiplyMM(
                viewProjectionMatrix, 0,
                perspectiveProjectionMatrix, 0,
                viewMatrix, 0
            );

            // drawing the grid
            y = 1.0f;
            for(int i = 0; i < 41; i++)
            {
                Matrix.setIdentityM(modelMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);

                if(i != 20)
                {
                    Matrix.translateM(modelMatrix, 0, 0.0f, y, 0.0f);
                    Matrix.multiplyMM(
                        modelViewProjectionMatrix, 0,
                        viewProjectionMatrix, 0,
                        modelMatrix, 0
                    );

                    GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
                    GLES32.glVertexAttrib4f(GLESView.KVD_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f, 1.0f);
                
                    GLES32.glBindVertexArray(vaoLine[0]);
                        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
                    GLES32.glBindVertexArray(0);
                }

                y = y - (1.0f / 20.0f);
            }

            x = 1.0f;
            for(int i = 0; i < 41; i++)
            {
                Matrix.setIdentityM(modelMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);

                if(i != 20)
                {
                    Matrix.translateM(modelMatrix, 0, x, 0.0f, 0.0f);
                    Matrix.rotateM(modelMatrix, 0, 90.0f, 0.0f, 0.0f, 1.0f);
                    Matrix.multiplyMM(
                        modelViewProjectionMatrix, 0,
                        viewProjectionMatrix, 0,
                        modelMatrix, 0
                    );

                    GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
                    GLES32.glVertexAttrib4f(GLESView.KVD_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f, 1.0f);
                
                    GLES32.glBindVertexArray(vaoLine[0]);
                        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
                    GLES32.glBindVertexArray(0);
                }

                x = x - (1.0f / 20.0f);
            }

            // X-Axis
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);
            Matrix.multiplyMM(
                modelViewProjectionMatrix, 0,
                viewProjectionMatrix, 0,
                modelMatrix, 0
            );
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
            GLES32.glVertexAttrib4f(GLESView.KVD_ATTRIBUTE_COLOR, 1.0f, 0.0f, 0.0f, 1.0f);
            GLES32.glBindVertexArray(vaoLine[0]);
                GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
            GLES32.glBindVertexArray(0);

            // Y-Axis
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);
            Matrix.rotateM(modelMatrix, 0, 90.0f, 0.0f, 0.0f, 1.0f);
            Matrix.multiplyMM(
                modelViewProjectionMatrix, 0,
                viewProjectionMatrix, 0,
                modelMatrix, 0
            );
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
            GLES32.glVertexAttrib4f(GLESView.KVD_ATTRIBUTE_COLOR, 0.0f, 1.0f, 0.0f, 1.0f);
            GLES32.glBindVertexArray(vaoLine[0]);
                GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
            GLES32.glBindVertexArray(0);

            // triangle
            if(showTriangle)
            {
                Matrix.setIdentityM(modelMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);
                Matrix.scaleM(modelMatrix, 0, 0.7f, 0.7f, 0.7f);
                Matrix.multiplyMM(
                    modelViewProjectionMatrix, 0,
                    viewProjectionMatrix, 0,
                    modelMatrix, 0
                );
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
                GLES32.glVertexAttrib4f(GLESView.KVD_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f, 1.0f);
                GLES32.glBindVertexArray(vaoTriangle[0]);
                    GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 3);
                GLES32.glBindVertexArray(0);
            }

            // square
            if(showSquare)
            {
                Matrix.setIdentityM(modelMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);
                Matrix.scaleM(modelMatrix, 0, 0.7f, 0.7f, 0.7f);
                Matrix.multiplyMM(
                    modelViewProjectionMatrix, 0,
                    viewProjectionMatrix, 0,
                    modelMatrix, 0
                );
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
                GLES32.glVertexAttrib4f(GLESView.KVD_ATTRIBUTE_COLOR, 0.8f, 0.3f, 0.0f, 1.0f);
                GLES32.glBindVertexArray(vaoSquare[0]);
                    GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 4);
                GLES32.glBindVertexArray(0);
            }

            // circle
            if(showCircle)
            {
                Matrix.setIdentityM(modelMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);
                Matrix.scaleM(modelMatrix, 0, 0.7f, 0.7f, 0.7f);
                Matrix.multiplyMM(
                    modelViewProjectionMatrix, 0,
                    viewProjectionMatrix, 0,
                    modelMatrix, 0
                );
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
                GLES32.glVertexAttrib4f(GLESView.KVD_ATTRIBUTE_COLOR, 1.0f, 0.0f, 1.0f, 1.0f);
                GLES32.glBindVertexArray(vaoCircle[0]);
                    GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 100);
                GLES32.glBindVertexArray(0);
            }
        }
        GLES32.glUseProgram(0);

        requestRender();  // swapping buffers
    }

    private void update() {
        /* no update needed */
    }

    private void uninitialize() {
        if(vboCircle[0] > 0) {
            GLES32.glDeleteBuffers(1, vboCircle, 0);
            vboCircle[0] = 0;
        }

        if(vaoCircle[0] > 0) {
            GLES32.glDeleteBuffers(1, vaoCircle, 0);
            vaoCircle[0] = 0;
        }

        if(vboSquare[0] > 0) {
            GLES32.glDeleteBuffers(1, vboSquare, 0);
            vboSquare[0] = 0;
        }

        if(vaoSquare[0] > 0) {
            GLES32.glDeleteBuffers(1, vaoSquare, 0);
            vaoSquare[0] = 0;
        }

        if(vboTriangle[0] > 0) {
            GLES32.glDeleteBuffers(1, vboTriangle, 0);
            vboTriangle[0] = 0;
        }

        if(vaoTriangle[0] > 0) {
            GLES32.glDeleteBuffers(1, vaoTriangle, 0);
            vaoTriangle[0] = 0;
        }

        if(vboLine[0] > 0) {
            GLES32.glDeleteBuffers(1, vboLine, 0);
            vboLine[0] = 0;
        }

        if(vaoLine[0] > 0) {
            GLES32.glDeleteBuffers(1, vaoLine, 0);
            vaoLine[0] = 0;
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
