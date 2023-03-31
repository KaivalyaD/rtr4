/*
 * Important notes for WebGL texture programs
 * These settings decide whether the desired output is visible or not:
 * 
 * Mozilla Firefox:
 * 1. Go to URL: 'about:config';
 * 2. Setting 'content.cors.disable' MUST be true;
 * 3. Setting 'security.fileuri.strict_origin_policy' MUST be false.
 * 
 * Google Chrome (or equivalently replace name by another chrome-based browser):
 * 1. Run terminal;
 * 2. Issue command:
 *      on linux: $google-chrome --disable-web-security --user-data-dir="path/to/dir"
 *      on Windows: >chrome.exe --disable-web-security --user-data-dir="path\to\dir"
 * 
 * Safari:
 * 1. Go to Preferences;
 * 2. Disable JavaScript Lock;
 * 3. Enable Dev Mode.
 */
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
var uniforms = {
    modelMatrix : null,
    viewMatrix : null,
    projMatrix : null,
    La : null,
    Ld : null,
    Ls : null,
    lightPosition: null,
    Ka : null,
    Kd : null,
    Ks : null,
    materialShininess : null,
    viewerPosition : null,
    isLight : null,
    texSampler: null
};

// lighting
var light = {
    ambient : null,
    diffuse : null,
    specular : null,
    position : null,
    isOn : null
};
var material = {
    ambient : null,
    diffuse : null,
    specular : null,
    shininess: null
};

// for the App
var vao = null;
var vboInterleaved = null;
var texMarble = null
var perspectiveProjectionMatrix = null;
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
    const texImageUrl = {
        marble : "./res/Marble.png"
    };

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
    in vec4 a_color;
    in vec3 a_normal;
    in vec2 a_texCoord;

    uniform mat4 modelMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 projMatrix;
    
    uniform vec4 lightPosition;  // in eye coordinates
    uniform vec4 viewerPosition; // in eye coordinates

    uniform int isLight;

    out vec4 a_color_out;
    out vec2 a_texCoord_out;
    out vec3 a_normal_out;
    out vec3 a_lightDirection_out;
    out vec3 a_viewDirection_out;

    void main(void)
    {
        mat4 mvMatrix = viewMatrix * modelMatrix;
        vec4 eyeCoordinate = mvMatrix * a_position;
        if(isLight != 0) {
            mat3 normalMatrix = mat3(transpose(inverse(mvMatrix)));
            a_normal_out = normalMatrix * a_normal;
            a_lightDirection_out = lightPosition.xyz - eyeCoordinate.xyz;
            a_viewDirection_out = viewerPosition.xyz - eyeCoordinate.xyz;
        }
        gl_Position = projMatrix * eyeCoordinate;
        a_color_out = a_color;
        a_texCoord_out = a_texCoord;
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

    in vec4 a_color_out;
    in vec2 a_texCoord_out;
    in vec3 a_normal_out;
    in vec3 a_lightDirection_out;
    in vec3 a_viewDirection_out;

    uniform vec4 La;
    uniform vec4 Ld;
    uniform vec4 Ls;

    uniform vec4 Ka;
    uniform vec4 Kd;
    uniform vec4 Ks;
    uniform float materialShininess;

    uniform int isLight;

    uniform sampler2D texSampler;

    out vec4 FragColor;

    void main(void)
    {
        vec4 finalColor;

        if(isLight != 0) {
            vec3 normal = normalize(a_normal_out);
            vec3 lightDirection = normalize(a_lightDirection_out);
            vec3 viewDirection = normalize(a_viewDirection_out);            
            vec3 reflectedDirection = reflect(-lightDirection, normal);

            vec4 ambient = La * Ka;
            vec4 diffuse = Ld * Kd * max(dot(normal, lightDirection), 0.0);
            vec4 specular = Ls * Ks * pow(max(dot(reflectedDirection, viewDirection), 0.0), materialShininess);

            finalColor = ambient + diffuse + specular;
        }
        else {
            finalColor = vec4(1.0);
        }

        finalColor *= a_color_out;
        finalColor *= texture(texSampler, a_texCoord_out);

        FragColor = finalColor;
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
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_COLOR, "a_color");  // perform pre-linking rituals
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_NORMAL, "a_normal");  // perform pre-linking rituals
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_TEXTURE0, "a_texCoord");  // perform pre-linking rituals
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
    uniforms.modelMatrix = gl.getUniformLocation(shaderProgramObject, "modelMatrix");
    uniforms.viewMatrix = gl.getUniformLocation(shaderProgramObject, "viewMatrix");
    uniforms.projMatrix = gl.getUniformLocation(shaderProgramObject, "projMatrix");
    uniforms.La = gl.getUniformLocation(shaderProgramObject, "La");
    uniforms.Ld = gl.getUniformLocation(shaderProgramObject, "Ld");
    uniforms.Ls = gl.getUniformLocation(shaderProgramObject, "Ls");
    uniforms.lightPosition = gl.getUniformLocation(shaderProgramObject, "lightPosition");
    uniforms.Ka = gl.getUniformLocation(shaderProgramObject, "Ka");
    uniforms.Kd = gl.getUniformLocation(shaderProgramObject, "Kd");
    uniforms.Ks = gl.getUniformLocation(shaderProgramObject, "Ks");
    uniforms.materialShininess = gl.getUniformLocation(shaderProgramObject, "materialShininess");
    uniforms.viewerPosition = gl.getUniformLocation(shaderProgramObject, "viewerPosition");
    uniforms.isLight = gl.getUniformLocation(shaderProgramObject, "isLight");
    uniforms.texSampler = gl.getUniformLocation(shaderProgramObject, "texSampler");

    // put vertex data into rendering pipeline
    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);
    {
        // vertex data
        vboInterleaved = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboInterleaved);
        {
            const cubeVertexDataInterleavedArray = Float32Array.from([
                // Positions          // Colors            // Normals             // TexCoords
                // Top face           // Top face          // Top face            // Top face
                1.0,  1.0, -1.0,      0.0, 0.0, 1.0,       0.0,  1.0,  0.0,       1.0, 1.0,
               -1.0,  1.0, -1.0,      0.0, 0.0, 1.0,       0.0,  1.0,  0.0,       0.0, 1.0,
               -1.0,  1.0,  1.0,      0.0, 0.0, 1.0,       0.0,  1.0,  0.0,       0.0, 0.0,
                1.0,  1.0,  1.0,      0.0, 0.0, 1.0,       0.0,  1.0,  0.0,       1.0, 0.0,

                // Bottom face        // Bottom face       // Bottom face         // Bottom face
                1.0, -1.0, -1.0,      0.0, 1.0, 0.0,       0.0, -1.0,  0.0,       1.0, 1.0,
                1.0, -1.0,  1.0,      0.0, 1.0, 0.0,       0.0, -1.0,  0.0,       0.0, 1.0,
               -1.0, -1.0,  1.0,      0.0, 1.0, 0.0,       0.0, -1.0,  0.0,       0.0, 0.0,
               -1.0, -1.0, -1.0,      0.0, 1.0, 0.0,       0.0, -1.0,  0.0,       1.0, 0.0,

                // Front face         // Front face        // Front face          // Front face
                1.0,  1.0,  1.0,      1.0, 0.0, 0.0,       0.0,  0.0,  1.0,       1.0, 1.0,
               -1.0,  1.0,  1.0,      1.0, 0.0, 0.0,       0.0,  0.0,  1.0,       0.0, 1.0,
               -1.0, -1.0,  1.0,      1.0, 0.0, 0.0,       0.0,  0.0,  1.0,       0.0, 0.0,
                1.0, -1.0,  1.0,      1.0, 0.0, 0.0,       0.0,  0.0,  1.0,       1.0, 0.0,

                // Back face          // Back face         // Back face           // Back face
                1.0,  1.0, -1.0,      0.0, 1.0, 1.0,       0.0,  0.0, -1.0,       1.0, 1.0,
                1.0, -1.0, -1.0,      0.0, 1.0, 1.0,       0.0,  0.0, -1.0,       0.0, 1.0,
               -1.0, -1.0, -1.0,      0.0, 1.0, 1.0,       0.0,  0.0, -1.0,       0.0, 0.0,
               -1.0,  1.0, -1.0,      0.0, 1.0, 1.0,       0.0,  0.0, -1.0,       1.0, 0.0,

                // Right face         // Right face        // Right face          // Right face
                1.0,  1.0, -1.0,      1.0, 0.0, 1.0,       1.0,  0.0,  0.0,       1.0, 1.0,
                1.0,  1.0,  1.0,      1.0, 0.0, 1.0,       1.0,  0.0,  0.0,       0.0, 1.0,
                1.0, -1.0,  1.0,      1.0, 0.0, 1.0,       1.0,  0.0,  0.0,       0.0, 0.0,
                1.0, -1.0, -1.0,      1.0, 0.0, 1.0,       1.0,  0.0,  0.0,       1.0, 0.0,

                // Left face          // Left face         // Left face           // Left face
                -1.0,  1.0,  1.0,     1.0, 1.0, 0.0,      -1.0,  0.0,  0.0,       1.0, 1.0,
                -1.0,  1.0, -1.0,     1.0, 1.0, 0.0,      -1.0,  0.0,  0.0,       0.0, 1.0,
                -1.0, -1.0, -1.0,     1.0, 1.0, 0.0,      -1.0,  0.0,  0.0,       0.0, 0.0,
                -1.0, -1.0,  1.0,     1.0, 1.0, 0.0,      -1.0,  0.0,  0.0,       1.0, 0.0
            ]);

            gl.bufferData(gl.ARRAY_BUFFER, cubeVertexDataInterleavedArray, gl.STATIC_DRAW);
            
            const sizeof_float = cubeVertexDataInterleavedArray.BYTES_PER_ELEMENT;
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 11 * sizeof_float, 0);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 11 * sizeof_float, 3 * sizeof_float);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 11 * sizeof_float, 6 * sizeof_float);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 11 * sizeof_float, 9 * sizeof_float);
            
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_COLOR);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_NORMAL);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_TEXTURE0);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    // load textures
    texMarble = loadGLTexture(texImageUrl.marble);
    if(!texMarble)
    {
        console.log("initialize(): failed to load texture at " + texImageUrl.marble);
        uninitialize();
    }

    // create and initialize perspective projection matrix
    perspectiveProjectionMatrix = mat4.create();
    mat4.identity(perspectiveProjectionMatrix);

    // setup light params
    light.ambient = [0.1, 0.1, 0.1, 1.0];
    light.diffuse = [1.0, 1.0, 1.0, 1.0];
    light.specular = [1.0, 1.0, 1.0, 1.0];
    light.position = [0.0, 0.0, 5.0, 1.0];

    // setup material params
    material.ambient = [1.0, 1.0, 1.0, 1.0];
    material.diffuse = [0.5, 0.5, 0.5, 1.0];
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

    theta_rad = theta * (Math.PI / 180.0);

    gl.useProgram(shaderProgramObject);
    {
        // perform transformations
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -6.0]);
        mat4.rotateX(modelMatrix, modelMatrix, theta_rad);
        mat4.rotateY(modelMatrix, modelMatrix, theta_rad);
        mat4.rotateZ(modelMatrix, modelMatrix, theta_rad);

        // set up uniforms
        gl.uniformMatrix4fv(uniforms.modelMatrix, false, modelMatrix);
        gl.uniformMatrix4fv(uniforms.viewMatrix, false, mat4.create());
        gl.uniformMatrix4fv(uniforms.projMatrix, false, perspectiveProjectionMatrix);
        gl.uniform4fv(uniforms.La, light.ambient);
        gl.uniform4fv(uniforms.Ld, light.diffuse);
        gl.uniform4fv(uniforms.Ls, light.specular);
        gl.uniform4fv(uniforms.lightPosition, light.position);
        gl.uniform4fv(uniforms.Ka, material.ambient);
        gl.uniform4fv(uniforms.Kd, material.diffuse);
        gl.uniform4fv(uniforms.Ks, material.specular);
        gl.uniform1f(uniforms.materialShininess, material.shininess);
        gl.uniform4fv(uniforms.viewerPosition, [0.0, 0.0, 4.5, 1.0]);
        gl.uniform1i(uniforms.isLight, light.isOn ? 1 : 0);

        // set active texture and bind
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texMarble);
        gl.uniform1i(uniforms.texSampler, 0);

        // drawing commands
        gl.bindVertexArray(vao);
            gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
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

    if(texMarble)
    {
        gl.deleteTexture(texMarble);
        texMarble = null;
    }
    
    if(vboInterleaved != null)
    {
        gl.deleteBuffer(vboInterleaved);
        vboInterleaved = null;
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

function loadGLTexture(texImageUrl) {
    // code
    let texture = gl.createTexture();

    gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    {
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);

        let bluePixel = new Uint8Array([0, 0, 255, 255]);
        gl.texImage2D(
            gl.TEXTURE_2D,
            0,
            gl.RGBA,
            1, 1,
            0,
            gl.RGBA,
            gl.UNSIGNED_BYTE,
            bluePixel
        );
    }
    gl.bindTexture(gl.TEXTURE_2D, null);

    let texImage = new Image();
    texImage.crossOrigin = "anonymous";
    texImage.onload = function() {
        gl.bindTexture(gl.TEXTURE_2D, texture);
        {
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);

            gl.texImage2D(
                gl.TEXTURE_2D,
                0,
                gl.RGBA,
                gl.RGBA,
                gl.UNSIGNED_BYTE,
                texImage
            );
            gl.generateMipmap(gl.TEXTURE_2D);
        }
        gl.bindTexture(gl.TEXTURE_2D, null);
    };
    texImage.src = texImageUrl;

    return texture;
}
