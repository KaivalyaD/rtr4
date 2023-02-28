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

    private int laUniform;
    private int ldUniform;
    private int lsUniform;
    private int lightPositionUniform;

    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int materialShininessUniform;

    private int lightEnabledUniform;

    private int modelViewMatrixUniform;
    private int projectionMatrixUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

    private boolean lightSwitch = false;
    private int numSphereVertices;
    private int numSphereElements;

    private int viewWidth = 0;
    private int viewHeight = 0;
    private int rotationAxis = 0;
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
        if(lightSwitch)
            rotationAxis = (rotationAxis + 1) % 4;
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
            "uniform vec4 u_lightPosition;  // in world space\n" +
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
            "\n" +
            "void main(void)\n" +
            "{\n" +
                "vec4 eyeCoordinate = u_modelViewMatrix * a_position;\n" +
                "\n" +
                "if(u_lightEnabled > 0)\n" +
                "{\n" +
                    "lightDirection = normalize(u_lightPosition.xyz - eyeCoordinate.xyz);\n" +
                    "\n" +
                    "mat3 normalMatrix = mat3(transpose(inverse(u_modelViewMatrix)));\n" +
                    "transformedNormal = normalize(normalMatrix * a_normal);\n" +
                    "\n" +
                    "viewerVector = -eyeCoordinate.xyz;\n" +
                "}\n" +
                "else {\n" +
                    "lightDirection = vec3(0.0);\n" +
                    "transformedNormal = vec3(0.0);\n" +
                    "viewerVector = vec3(0.0);\n" +
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
            "in vec3 lightDirection;\n" +
            "in vec3 transformedNormal;\n" +
            "in vec3 viewerVector;\n" +
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
                    "FragColor = ambient + diffuse + specular;\n" +
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

        // get all uniform locations
        laUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_La");
        ldUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld");
        lsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ls");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lightPosition");

        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ka");
        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ks");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_materialShininess");

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

        GLES32.glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    }

    private void resize(int width, int height) {
        height = (height <= 0 ? 1 : height);

        viewWidth = width;
        viewHeight = height;
        GLES32.glViewport(0, 0, width, height);

        float aspectRatio = (float)(width / 4) / (float)(height / 6);  /* essential change for 24 spheres!!! */
        Matrix.perspectiveM(
            perspectiveProjectionMatrix, 0,
            45.0f,
            aspectRatio,
            0.1f,
            100.0f
        );
    }

    private void render() {
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        draw24Spheres();

        requestRender();  // swapping buffers
    }

    private void update() {
        if(!lightSwitch)
            return;
        
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

    private void setMaterial(float[] Ka, float[] Kd, float[] Ks, float shininess) {
        GLES32.glUniform4f(kaUniform, Ka[0], Ka[1], Ka[2], 1.0f);
        GLES32.glUniform4f(kdUniform, Kd[0], Kd[1], Kd[2], 1.0f);
        GLES32.glUniform4f(ksUniform, Ks[0], Ks[1], Ks[2], 1.0f);
        GLES32.glUniform1f(materialShininessUniform, shininess);
    }

    private void setVec3(float[] vec3, float x, float y, float z) {
        vec3[0] = x;
        vec3[1] = y;
        vec3[2] = z;
    }

    private void draw24Spheres() {
        final float[] ambientLightColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        final float[] diffuseLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        final float[] specularLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        float[] lightPosition = new float[4];

        float[] modelViewMatrix = new float[16];

        float[] Ka = new float[3];
        float[] Kd = new float[3];
        float[] Ks = new float[3];

        if(rotationAxis == 1) {
            lightPosition[0] = 0.0f;
            lightPosition[1] = 10.0f * (float)Math.sin(Math.toRadians((double)theta));
            lightPosition[2] = 10.0f * (float)Math.cos(Math.toRadians((double)theta));
            lightPosition[3] = 1.0f;
        }
        else if(rotationAxis == 2) {
            lightPosition[0] = 10.0f * (float)Math.cos(Math.toRadians((double)theta));
            lightPosition[1] = 0.0f;
            lightPosition[2] = 10.0f * (float)Math.sin(Math.toRadians((double)theta));
            lightPosition[3] = 1.0f;
        }
        else if(rotationAxis == 3) {
            lightPosition[0] = 10.0f * (float)Math.cos(Math.toRadians((double)theta));
            lightPosition[1] = 10.0f * (float)Math.sin(Math.toRadians((double)theta));
            lightPosition[2] = 0.0f;
            lightPosition[3] = 1.0f;
        }
        else {
            lightPosition[0] = 0.0f;
            lightPosition[1] = 0.0f;
            lightPosition[2] = 10.0f;
            lightPosition[3] = 1.0f;
        }

        GLES32.glUseProgram(shaderProgramObject);
        {
            Matrix.setLookAtM(
                modelViewMatrix, 0,
                0.0f, 0.0f, 1.5f,
                0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f
            );
            GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);
            GLES32.glUniformMatrix4fv(modelViewMatrixUniform, 1, false, modelViewMatrix, 0);
            
            GLES32.glBindVertexArray(vaoSphere[0]);
            {
                if(lightSwitch)
                {
                    GLES32.glUniform4fv(laUniform, 1, ambientLightColor, 0);
                    GLES32.glUniform4fv(ldUniform, 1, diffuseLightColor, 0);
                    GLES32.glUniform4fv(lsUniform, 1, specularLightColor, 0);
                    GLES32.glUniform4fv(lightPositionUniform, 1, lightPosition, 0);
                    GLES32.glUniform1i(lightEnabledUniform, 1);

                    // [1, 1] material: emerald
                    GLES32.glViewport(0, 5 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0215f, 0.1745f, 0.0215f);
                    setVec3(Kd, 0.07568f, 0.61424f, 0.07568f);
                    setVec3(Ks, 0.633f, 0.727811f, 0.633f);
                    setMaterial(Ka, Kd, Ks, 0.6f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [2, 1] material: jade
                    GLES32.glViewport(0, 4 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.135f, 0.2225f, 0.1575f);
                    setVec3(Kd, 0.54f, 0.89f, 0.63f);
                    setVec3(Ks, 0.316228f, 0.316228f, 0.316228f);
                    setMaterial(Ka, Kd, Ks, 0.1f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [3, 1] material: obsidian
                    GLES32.glViewport(0, 3 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.05375f, 0.05f, 0.06625f);
                    setVec3(Kd, 0.18275f, 0.17f, 0.22525f);
                    setVec3(Ks, 0.332741f, 0.328634f, 0.346435f);
                    setMaterial(Ka, Kd, Ks, 0.3f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);
                
                    // [4, 1] material: pearl
                    GLES32.glViewport(0, 2 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.25f, 0.20725f, 0.20725f);
                    setVec3(Kd, 1.0f, 0.829f, 0.829f);
                    setVec3(Ks, 0.296648f, 0.296648f, 0.296648f);
                    setMaterial(Ka, Kd, Ks, 0.3f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [5, 1] material: ruby
                    GLES32.glViewport(0, viewHeight / 6, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.1745f, 0.01175f, 0.01175f);
                    setVec3(Kd, 0.61424f, 0.04136f, 0.04136f);
                    setVec3(Ks, 0.727811f, 0.727811f, 0.727811f);
                    setMaterial(Ka, Kd, Ks, 0.6f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [6, 1] material: turquoise
                    GLES32.glViewport(0, 0, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.1f, 0.18725f, 0.1745f);
                    setVec3(Kd, 0.396f, 0.74151f, 0.69102f);
                    setVec3(Ks, 0.297254f, 0.30829f, 0.306678f);
                    setMaterial(Ka, Kd, Ks, 0.1f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [1, 2] material: brass
                    GLES32.glViewport(viewWidth / 4, 5 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.329412f, 0.223529f, 0.027451f);
                    setVec3(Kd, 0.780392f, 0.568627f, 0.113725f);
                    setVec3(Ks, 0.992157f, 0.941176f, 0.807843f);
                    setMaterial(Ka, Kd, Ks, 0.21794872f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [2, 2] material: bronze
                    GLES32.glViewport(viewWidth / 4, 4 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.2125f, 0.1275f, 0.054f);
                    setVec3(Kd, 0.714f, 0.4284f, 0.18144f);
                    setVec3(Ks, 0.393548f, 0.271906f, 0.166721f);
                    setMaterial(Ka, Kd, Ks, 0.2f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [3, 2] material: chrome
                    GLES32.glViewport(viewWidth / 4, 3 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.25f, 0.25f, 0.25f);
                    setVec3(Kd, 0.4f, 0.4f, 0.4f);
                    setVec3(Ks, 0.774597f, 0.774597f, 0.774597f);
                    setMaterial(Ka, Kd, Ks, 0.6f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);
                
                    // [4, 2] material: copper
                    GLES32.glViewport(viewWidth / 4, 2 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.19125f, 0.0735f, 0.0225f);
                    setVec3(Kd, 0.7038f, 0.27048f, 0.0828f);
                    setVec3(Ks, 0.256777f, 0.137622f, 0.086014f);
                    setMaterial(Ka, Kd, Ks, 0.1f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [5, 2] material: gold
                    GLES32.glViewport(viewWidth / 4, viewHeight / 6, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.24725f, 0.1995f, 0.0745f);
                    setVec3(Kd, 0.75164f, 0.60648f, 0.22648f);
                    setVec3(Ks, 0.628281f, 0.555802f, 0.366065f);
                    setMaterial(Ka, Kd, Ks, 0.4f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [6, 2] material: silver
                    GLES32.glViewport(viewWidth / 4, 0, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.19225f, 0.19225f, 0.19225f);
                    setVec3(Kd, 0.50754f, 0.50754f, 0.50754f);
                    setVec3(Ks, 0.508273f, 0.508273f, 0.508273f);
                    setMaterial(Ka, Kd, Ks, 0.4f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [1, 3] material: black plastic
                    GLES32.glViewport(2 * (viewWidth / 4), 5 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.0f, 0.0f);
                    setVec3(Kd, 0.01f, 0.01f, 0.01f);
                    setVec3(Ks, 0.5f, 0.5f, 0.5f);
                    setMaterial(Ka, Kd, Ks, 0.25f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [2, 3] material: cyan plastic
                    GLES32.glViewport(2 * (viewWidth / 4), 4 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.1f, 0.06f);
                    setVec3(Kd, 0.0f, 0.50980392f, 0.50980392f);
                    setVec3(Ks, 0.50196078f, 0.50196078f, 0.50196078f);
                    setMaterial(Ka, Kd, Ks, 0.25f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [3, 3] material: green plastic
                    GLES32.glViewport(2 * (viewWidth / 4), 3 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.0f, 0.0f);
                    setVec3(Kd, 0.1f, 0.35f, 0.1f);
                    setVec3(Ks, 0.45f, 0.55f, 0.45f);
                    setMaterial(Ka, Kd, Ks, 0.25f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [4, 3] material: red plastic
                    GLES32.glViewport(2 * (viewWidth / 4), 2 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.0f, 0.0f);
                    setVec3(Kd, 0.5f, 0.0f, 0.0f);
                    setVec3(Ks, 0.7f, 0.6f, 0.6f);
                    setMaterial(Ka, Kd, Ks, 0.25f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [5, 3] material: white plastic
                    GLES32.glViewport(2 * (viewWidth / 4), viewHeight / 6, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.0f, 0.0f);
                    setVec3(Kd, 0.55f, 0.55f, 0.55f);
                    setVec3(Ks, 0.7f, 0.7f, 0.7f);
                    setMaterial(Ka, Kd, Ks, 0.25f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [6, 3] material: yellow plastic
                    GLES32.glViewport(2 * (viewWidth / 4), 0, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.0f, 0.0f);
                    setVec3(Kd, 0.5f, 0.5f, 0.0f);
                    setVec3(Ks, 0.6f, 0.6f, 0.5f);
                    setMaterial(Ka, Kd, Ks, 0.25f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [1, 4] material: black rubber
                    GLES32.glViewport(3 * (viewWidth / 4), 5 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.02f, 0.02f, 0.02f);
                    setVec3(Kd, 0.01f, 0.01f, 0.01f);
                    setVec3(Ks, 0.4f, 0.4f, 0.4f);
                    setMaterial(Ka, Kd, Ks, 0.078125f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);
                
                    // [2, 4] material: cyan rubber
                    GLES32.glViewport(3 * (viewWidth / 4), 4 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.05f, 0.05f);
                    setVec3(Kd, 0.4f, 0.5f, 0.5f);
                    setVec3(Ks, 0.04f, 0.7f, 0.7f);
                    setMaterial(Ka, Kd, Ks, 0.078125f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [3, 4] material: green rubber
                    GLES32.glViewport(3 * (viewWidth / 4), 3 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.0f, 0.05f, 0.05f);
                    setVec3(Kd, 0.4f, 0.5f, 0.4f);
                    setVec3(Ks, 0.04f, 0.7f, 0.04f);
                    setMaterial(Ka, Kd, Ks, 0.078125f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [4, 4] material: red rubber
                    GLES32.glViewport(3 * (viewWidth / 4), 2 * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.05f, 0.0f, 0.0f);
                    setVec3(Kd, 0.5f, 0.4f, 0.4f);
                    setVec3(Ks, 0.7f, 0.04f, 0.04f);
                    setMaterial(Ka, Kd, Ks, 0.078125f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [5, 4] material: white rubber
                    GLES32.glViewport(3 * (viewWidth / 4), viewHeight / 6, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.05f, 0.05f, 0.05f);
                    setVec3(Kd, 0.5f, 0.5f, 0.5f);
                    setVec3(Ks, 0.7f, 0.7f, 0.7f);
                    setMaterial(Ka, Kd, Ks, 0.078125f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);

                    // [6, 4] material: yellow rubber
                    GLES32.glViewport(3 * (viewWidth / 4), 0, viewWidth / 4, viewHeight / 6);
                    setVec3(Ka, 0.05f, 0.05f, 0.0f);
                    setVec3(Kd, 0.5f, 0.5f, 0.4f);
                    setVec3(Ks, 0.7f, 0.7f, 0.04f);
                    setMaterial(Ka, Kd, Ks, 0.078125f * 128.0f);
                    GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                    GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);
                }
                else
                {
                    GLES32.glUniform1i(lightEnabledUniform, 0);
                    
                    for(int row = 0; row < 6; row++)
                    {
                        for(int col = 0; col < 4; col++)
                        {
                            GLES32.glViewport(col * (viewWidth / 4), row * (viewHeight / 6), viewWidth / 4, viewHeight / 6);
                            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vboSphereElements[0]);
                            GLES32.glDrawElements(GLES32.GL_TRIANGLES, numSphereElements, GLES32.GL_UNSIGNED_SHORT, 0);
                        }
                    }
                }
            }
            GLES32.glBindVertexArray(0);
        }
        GLES32.glUseProgram(0);
    }
}
