package com.kayd.window;

// declaration of a light
public class GLLight {
    private float[] ambientColor;
    private float[] diffuseColor;
    private float[] specularColor;
    private float[] position;

    private int ambientColorUniformLocation;
    private int diffuseColorUniformLocation;
    private int specularColorUniformLocation;
    private int positionUniformLocation;

    private int switchUniformLocation;

    public GLLight() {
        this.ambientColor = new float[4];
        this.diffuseColor = new float[4];
        this.specularColor = new float[4];
        this.position = new float[4];

        ambientColorUniformLocation = -1;
        diffuseColorUniformLocation = -1;
        specularColorUniformLocation = -1;
        positionUniformLocation = -1;
    }

    public void setAmbientColor(float r, float g, float b, float a) {
        this.ambientColor[0] = r;
        this.ambientColor[1] = g;
        this.ambientColor[2] = b;
        this.ambientColor[3] = a;
    }

    public void setDiffuseColor(float r, float g, float b, float a) {
        this.diffuseColor[0] = r;
        this.diffuseColor[1] = g;
        this.diffuseColor[2] = b;
        this.diffuseColor[3] = a;
    }

    public void setSpecularColor(float r, float g, float b, float a) {
        this.specularColor[0] = r;
        this.specularColor[1] = g;
        this.specularColor[2] = b;
        this.specularColor[3] = a;
    }

    public void setPosition(float x, float y, float z, float w) {
        this.position[0] = x;
        this.position[1] = y;
        this.position[2] = z;
        this.position[3] = w;
    }

    public float[] getAmbientColor() {
        return this.ambientColor;
    }

    public float[] getDiffuseColor() {
        return this.diffuseColor;
    }

    public float[] getSpecularColor() {
        return this.specularColor;
    }

    public float[] getPosition() {
        return this.position;
    }

    public void setAmbientColorUniformLocation(int location) {
        if(location >= 0)
            this.ambientColorUniformLocation = location;
        else
            System.out.println("KVD: GLLight.setAmbientColorUniformLocation(): uniform location < 0 is invalid");
    }

    public void setDiffuseColorUniformLocation(int location) {
        if(location >= 0)
            this.diffuseColorUniformLocation = location;
        else
            System.out.println("KVD: GLLight.setDiffuseColorUniformLocation(): uniform location < 0 is invalid");
    }

    public void setSpecularColorUniformLocation(int location) {
        if(location >= 0)
            this.specularColorUniformLocation = location;
        else
            System.out.println("KVD: GLLight.setSpecularColorUniformLocation(): uniform location < 0 is invalid");
    }

    public void setPositionUniformLocation(int location) {
        if(location >= 0)
            this.positionUniformLocation = location;
        else
            System.out.println("KVD: GLLight.setPositionUniformLocation(): uniform location < 0 is invalid");
    }

    public int getAmbientColorUniformLocation() {
        return this.ambientColorUniformLocation;
    }

    public int getDiffuseColorUniformLocation() {
        return this.diffuseColorUniformLocation;
    }

    public int getSpecularColorUniformLocation() {
        return this.specularColorUniformLocation;
    }

    public int getPositionUniformLocation() {
        return this.positionUniformLocation;
    }

    public void setSwitchUniformLocation(int location) {
        if(location > 0)
            this.switchUniformLocation = location;
        else
            System.out.println("KVD: GLLight.setSwitchUniformLocation(): uniform location < 0 is invalid");
    }

    public int getSwitchUniformLocation() {
        return this.switchUniformLocation;
    }
}
