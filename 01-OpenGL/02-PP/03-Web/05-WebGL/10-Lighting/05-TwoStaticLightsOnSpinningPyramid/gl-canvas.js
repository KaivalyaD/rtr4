var canvas = null;
var gl = null;
var isFullscreen = false;
var canvasOriginalWidth, canvasOriginalHeight;

// for uniformity among programs
const KVD = {
    ATTRIBUTE_POSITION : 0,
    ATTRIBUTE_COLOR : 1,
    ATTRIBUTE_NORMAL : 2,
    ATTRIBUTE_TEXTURE0 : 3
};

// for GL
var shaderProgramObject;

// uniforms
class LightingUniforms {
    modelMatrix;
    viewMatrix;
    projMatrix;
    La;
    Ld;
    Ls;
    lightPosition;
    Ka;
    Kd;
    Ks;
    materialShininess;
    viewerPosition;
    isLight;
    constructor() {
        this.La = [];
        this.Ld = [];
        this.Ls = [];
        this.lightPosition = [];
    }
};

// lighting
class Light {
    ambient;
    diffuse;
    specular;
    position;
    isOn;
    constructor() {
        this.ambient = [];
        this.diffuse = [];
        this.specular = [];
        this.position = [];
    }
};
var material = {
    ambient : null,
    diffuse : null,
    specular : null,
    shininess: null
};

// for rendered objects
var vao;
var vboPositions;
var vboNormals;

// for the App
var uniforms;
var light;
var perspectiveProjectionMatrix;
var theta = 0.0;

// requestAnimationFrame() (browser-specific buffer-swapping function)
var requestAnimationFrame =
    window.requestAnimationFrame ||     // google chrome
    window.mozRequestAnimationFrame ||  // firefox
    window.oRequestAnimationFrame ||    // opera
    window.msRequestAnimationFrame ||   // ie/edge
    window.webkitRequestAnimationFrame; // safari

function main() {
    // code
    canvas = document.getElementById("WebGLCanvas");
    if(!canvas)
        console.log("main(): failed to obtain canvas from gl-canvas.html\n");
    else
        console.log("main(): successfully obtained canvas from gl-canvas.html\n");

    // record canvas dimensions for future resize() calls
    canvasOriginalWidth = canvas.width;
    canvasOriginalHeight = canvas.height;

    initialize(); // initialize WebGL
    resize();     // warm-up resize()

    // first render() call (no game loop in JS, instead cleverly use that of the browser)
    render();
    
    // add listeners that allow event to bubble up the hierarchy
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize, false);
}

/* keyboard event listener */
function keyDown(event) {
    // code
    switch(event.keyCode)
    {
    case 81:   // Q (esc is already used by browsers to toggle back from fullscreen mode)
    case 113:  // q
        uninitialize();
        window.close();  // try exiting the canvas' window (not all browsers like this)
        break;

    case 70:  // F
    case 102: // f
        toggleFullscreen();
        break;

    case 76:  // L
    case 108: // l
        light.isOn = !light.isOn;
        break;
    
    default:
        break;
    }
}

/* mouse event listener */
function mouseDown() {
    // code
}

/* fullscreen toggler */
function toggleFullscreen() {
    // code
    // 
    var fullscreen_element =
        document.fullscreenElement ||         // chrome/opera
        document.mozFullScreenElement ||      // firefox
        document.webkitFullscreenElement ||   // safari
        document.msFullscreenElement ||       // ie/edge
        null;                                 // already fullscreen

    if(!fullscreen_element)
    {
        if(canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if(canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if(canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if(canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
        
        isFullscreen = true;
    }
    else
    {
        if(document.exitFullscreen)
            document.exitFullscreen();
        else if(document.mozExitFullScreen)
            document.mozExitFullScreen();
        else if(document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();

        isFullscreen = false;
    }
}

function initialize() {
    // variable declarations
    var status;
    var infoLog;

    // code
    // obtain WebGL 2.0 context
    gl = canvas.getContext("webgl2");
    if(!gl)
        console.log("initialize(): failed to obtain WebGL 2.0 context from canvas\n");
    else
        console.log("initialize(): successfully obtained WebGL 2.0 context from canvas\n");
    
    // set context viewport width & height: an original idiosynchracy of WebGL
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    // log GL information
    logGLInfo();

    // vertex shader
    status = false;
    infoLog = "";

    var vertexShaderSource =
    `#version 300 es

    precision lowp int;
    precision mediump float;

    in vec4 a_position;
    in vec3 a_normal;

    uniform mat4 modelMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 projMatrix;
    
    uniform vec4 lightPosition[2];  // in eye coordinates
    uniform vec4 viewerPosition; // in eye coordinates

    uniform int isLight;

    out vec3 a_normal_out;
    out vec3 a_lightDirection_out[2];
    out vec3 a_viewDirection_out;

    void main(void)
    {
        mat4 mvMatrix = viewMatrix * modelMatrix;
        vec4 eyeCoordinate = mvMatrix * a_position;
        if(isLight != 0) {
            mat3 normalMatrix = mat3(transpose(inverse(mvMatrix)));
            a_normal_out = normalMatrix * a_normal;
            a_lightDirection_out[0] = lightPosition[0].xyz - eyeCoordinate.xyz;
            a_lightDirection_out[1] = lightPosition[1].xyz - eyeCoordinate.xyz;
            a_viewDirection_out = viewerPosition.xyz - eyeCoordinate.xyz;
        }
        gl_Position = projMatrix * eyeCoordinate;
    }`;

    var vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject, vertexShaderSource);
    gl.compileShader(vertexShaderObject);
    status = gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS);
    if(!status)
    {
        infoLog = gl.getShaderInfoLog(vertexShaderObject);
        console.log("initialize(): *** vertex shader compilation errors ***\n");
        console.log(infoLog);

        uninitialize();
    }
    else
        console.log("initialize(): vertex shader was compiled successfully\n");

    // fragment shader
    status = false;
    infoLog = "";

    var fragmentShaderSource = 
    `#version 300 es
    
    precision lowp int;
    precision mediump float;

    in vec3 a_normal_out;
    in vec3 a_lightDirection_out[2];
    in vec3 a_viewDirection_out;

    uniform vec4 La[2];
    uniform vec4 Ld[2];
    uniform vec4 Ls[2];

    uniform vec4 Ka;
    uniform vec4 Kd;
    uniform vec4 Ks;
    uniform float materialShininess;

    uniform int isLight;

    out vec4 FragColor;

    void main(void)
    {
        if(isLight != 0) {
            vec3 normal = normalize(a_normal_out);

            vec3 lightDirection[2];
            lightDirection[0] = normalize(a_lightDirection_out[0]);
            lightDirection[1] = normalize(a_lightDirection_out[1]);

            vec3 reflectedDirection[2];
            reflectedDirection[0] = reflect(-lightDirection[0], normal);
            reflectedDirection[1] = reflect(-lightDirection[1], normal);

            vec3 viewDirection = normalize(a_viewDirection_out);

            vec4 ambient = (La[0] + La[1]) * Ka;

            vec4 diffuse = vec4(0.0);
            diffuse += Ld[0] * Kd * max(dot(normal, lightDirection[0]), 0.0);
            diffuse += Ld[1] * Kd * max(dot(normal, lightDirection[1]), 0.0);
            
            vec4 specular = vec4(0.0);
            specular += Ls[0] * Ks * pow(max(dot(reflectedDirection[0], viewDirection), 0.0), materialShininess);
            specular += Ls[1] * Ks * pow(max(dot(reflectedDirection[1], viewDirection), 0.0), materialShininess);

            FragColor = ambient + diffuse + specular;
        }
        else {
            FragColor = vec4(1.0);
        }
    }`;

    var fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSource);
    gl.compileShader(fragmentShaderObject);
    status = gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS);
    if(!status)
    {
        infoLog = gl.getShaderInfoLog(fragmentShaderObject);
        console.log("initialize(): *** fragment shader compilation errors ***\n");
        console.log(infoLog);

        uninitialize();
    }
    else
        console.log("initialize(): fragment shader was compiled successfully\n");

    // shader program object
    status = false;
    infoLog = "";

    shaderProgramObject = gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_POSITION, "a_position");  // perform pre-linking rituals
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_NORMAL, "a_normal");  // perform pre-linking rituals
    gl.linkProgram(shaderProgramObject);  // link the program
    status = gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS);
    if(!status)
    {
        infoLog = gl.getProgramInfoLog(shaderProgramObject);
        console.log("initialize(): *** there were linking errors ***\n");
        console.log(infoLog + "\n");

        uninitialize();
    }
    else
        console.log("initialize(): shader program was linked successfully\n");

    // perform post-linking rituals
    uniforms = new LightingUniforms();
    uniforms.modelMatrix = gl.getUniformLocation(shaderProgramObject, "modelMatrix");
    uniforms.viewMatrix = gl.getUniformLocation(shaderProgramObject, "viewMatrix");
    uniforms.projMatrix = gl.getUniformLocation(shaderProgramObject, "projMatrix");
    uniforms.La[0] = gl.getUniformLocation(shaderProgramObject, "La[0]");
    uniforms.La[1] = gl.getUniformLocation(shaderProgramObject, "La[1]");
    uniforms.Ld[0] = gl.getUniformLocation(shaderProgramObject, "Ld[0]");
    uniforms.Ld[1] = gl.getUniformLocation(shaderProgramObject, "Ld[1]");
    uniforms.Ls[0] = gl.getUniformLocation(shaderProgramObject, "Ls[0]");
    uniforms.Ls[1] = gl.getUniformLocation(shaderProgramObject, "Ls[1]");
    uniforms.lightPosition[0] = gl.getUniformLocation(shaderProgramObject, "lightPosition[0]");
    uniforms.lightPosition[1] = gl.getUniformLocation(shaderProgramObject, "lightPosition[1]");
    uniforms.Ka = gl.getUniformLocation(shaderProgramObject, "Ka");
    uniforms.Kd = gl.getUniformLocation(shaderProgramObject, "Kd");
    uniforms.Ks = gl.getUniformLocation(shaderProgramObject, "Ks");
    uniforms.materialShininess = gl.getUniformLocation(shaderProgramObject, "materialShininess");
    uniforms.viewerPosition = gl.getUniformLocation(shaderProgramObject, "viewerPosition");
    uniforms.isLight = gl.getUniformLocation(shaderProgramObject, "isLight");

    // put geometry data into rendering pipeline
    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);
    {
        // positions vbo
        vboPositions = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboPositions);
        {
            const vertexPositionsTypedArray = new Float32Array([
                0.0,  1.0,  0.0,
               -1.0, -1.0,  1.0,
                1.0, -1.0,  1.0,

                // right
                0.0,  1.0,  0.0,
                1.0, -1.0,  1.0,
                1.0, -1.0, -1.0,

                // rear
                0.0,  1.0,  0.0,
                1.0, -1.0, -1.0,
               -1.0, -1.0, -1.0,

                // left
                0.0,  1.0,  0.0,
               -1.0, -1.0, -1.0,
               -1.0, -1.0,  1.0
            ]);

            gl.bufferData(gl.ARRAY_BUFFER, vertexPositionsTypedArray, gl.STATIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        // normals vbo
        vboNormals = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboNormals);
        {
            const vertexNormalsTypedArray = new Float32Array([
                0.0, 0.447214, 0.894427, // front-top
                0.0, 0.447214, 0.894427, // front-left
                0.0, 0.447214, 0.894427, // front-right

                0.894427, 0.447214, 0.0, // right-top
                0.894427, 0.447214, 0.0, // right-left
                0.894427, 0.447214, 0.0, // right-right

                0.0, 0.447214, -0.894427, // rear-top
                0.0, 0.447214, -0.894427, // rear-left
                0.0, 0.447214, -0.894427, // rear-right

               -0.894427, 0.447214, 0.0, // left-top
               -0.894427, 0.447214, 0.0, // left-left
               -0.894427, 0.447214, 0.0  // left-right
            ]);

            gl.bufferData(gl.ARRAY_BUFFER, vertexNormalsTypedArray, gl.STATIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_NORMAL);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    // create and initialize perspective projection matrix
    perspectiveProjectionMatrix = mat4.create();
    mat4.identity(perspectiveProjectionMatrix);

    // setup light params
    light = new Light();
    light.ambient[0] = [0.0, 0.0, 0.0, 1.0];
    light.diffuse[0] = [1.0, 0.0, 0.0, 1.0];
    light.specular[0] = [1.0, 0.0, 0.0, 1.0];
    light.position[0] = [-2.0, 0.0, 0.0, 1.0];

    light.ambient[1] = [0.0, 0.0, 0.0, 1.0];
    light.diffuse[1] = [0.0, 0.0, 1.0, 1.0];
    light.specular[1] = [0.0, 0.0, 1.0, 1.0];
    light.position[1] = [2.0, 0.0, 0.0, 1.0];

    // setup material params
    material.ambient = [0.0, 0.0, 0.0, 1.0];
    material.diffuse = [0.5, 0.2, 0.7, 1.0];
    material.specular = [0.7, 0.7, 0.7, 1.0];
    material.shininess = 128.0;

    // set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    // enable depth testing
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.enable(gl.CULL_FACE);
}

function logGLInfo() {
    // code
    document.getElementById("webgl_vendor_string").innerText = "Vendor: " + gl.getParameter(gl.VENDOR);
    document.getElementById("webgl_version_string").innerText = "Version: " + gl.getParameter(gl.VERSION);
    document.getElementById("webgl_sl_version_string").innerText = "GLSL Version: " + gl.getParameter(gl.SHADING_LANGUAGE_VERSION);
    document.getElementById("webgl_renderer_string").innerText = "Renderer: " + gl.getParameter(gl.RENDERER);
}

function resize() {
    // code
    if(isFullscreen)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width = canvasOriginalWidth;
        canvas.height = canvasOriginalHeight;
    }

    gl.viewport(0, 0, canvas.width, canvas.height);

    let aspectRatio = canvas.width / canvas.height;
    mat4.perspective(perspectiveProjectionMatrix, Math.PI / 4, aspectRatio, 0.0, 100.0);
}

function render() {
    // local variables
    var modelMatrix = mat4.create();

    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);
    {
        // perform transformations
        mat4.identity(modelMatrix);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.5]);
        mat4.rotateY(modelMatrix, modelMatrix, theta * (Math.PI / 180.0));

        // set up uniforms
        gl.uniformMatrix4fv(uniforms.modelMatrix, false, modelMatrix);
        gl.uniformMatrix4fv(uniforms.viewMatrix, false, mat4.create());
        gl.uniformMatrix4fv(uniforms.projMatrix, false, perspectiveProjectionMatrix);
        gl.uniform4fv(uniforms.La[0], light.ambient[0]);
        gl.uniform4fv(uniforms.Ld[0], light.diffuse[0]);
        gl.uniform4fv(uniforms.Ls[0], light.specular[0]);
        gl.uniform4fv(uniforms.lightPosition[0], light.position[0]);
        gl.uniform4fv(uniforms.La[1], light.ambient[1]);
        gl.uniform4fv(uniforms.Ld[1], light.diffuse[1]);
        gl.uniform4fv(uniforms.Ls[1], light.specular[1]);
        gl.uniform4fv(uniforms.lightPosition[1], light.position[1]);
        gl.uniform4fv(uniforms.Ka, material.ambient);
        gl.uniform4fv(uniforms.Kd, material.diffuse);
        gl.uniform4fv(uniforms.Ks, material.specular);
        gl.uniform1f(uniforms.materialShininess, material.shininess);
        gl.uniform4fv(uniforms.viewerPosition, [0.0, 0.0, 4.5, 1.0]);
        gl.uniform1i(uniforms.isLight, light.isOn ? 1 : 0);

        // drawing commands
        gl.bindVertexArray(vao);
            gl.drawArrays(gl.TRIANGLES, 0, 12);
        gl.bindVertexArray(null);
    }
    gl.useProgram(null);

    // swap buffers and maintain the rendering loop
    requestAnimationFrame(render, canvas);

    // update the frame
    update();
}

function update() {
    // code
    theta += 1.0;
    if(theta >= 360.0)
        theta -= 360.0;
}

function uninitialize() {
    // code
    if(isFullscreen)
        toggleFullscreen();

    if(vboNormals != null)
    {
        gl.deleteBuffer(vboNormals);
        vboNormals = null;
    }

    if(vboPositions != null)
    {
        gl.deleteBuffer(vboPositions);
        vboPositions = null;
    }

    if(vao != null)
    {
        gl.deleteVertexArray(vao);
        vao = null;
    }

    if(shaderProgramObject != null)
    {
        let attachedShadersCount = gl.getProgramParameter(shaderProgramObject, gl.ATTACHED_SHADERS);
        if(attachedShadersCount > 0)
        {
            let attachedShadersList = gl.getAttachedShaders(shaderProgramObject);
            let reversedAttachedShadersList = attachedShadersList.reverse();

            for(let i = 0; i < attachedShadersCount; i++)
            {
                gl.detachShader(shaderProgramObject, reversedAttachedShadersList[i]);
                gl.deleteShader(reversedAttachedShadersList[i]);
                reversedAttachedShadersList[i] = null;
            }
        }
        
        console.log("uninitialize(): detached and deleted " + attachedShadersCount + " shader objects\n");
        
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}
