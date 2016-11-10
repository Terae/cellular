//
// Created by benji on 07/11/16.
//

#include <glad/glad.h>

#include "Material.h"
#include "Context.h"

Material::Material(float diffuseR, float diffuseG, float diffuseB, float specularR, float specularG, float specularB,
                   float ambientR, float ambientG, float ambientB) :

        _ambientR(ambientR), _ambientG(ambientG), _ambientB(ambientB), _diffuseR(diffuseR), _diffuseG(diffuseG), _diffuseB(diffuseB),
        _specularR(specularR), _specularG(specularG), _specularB(specularB), _specularIntensity(0.5), _specularHardness(5) {

}

void Material::removeAllTextures() {
    this->textures.clear();
}

void Material::addTexture(Texture texture) {
    this->textures.push_back(texture);
}

void Material::setDiffuse(float r, float g, float b) {
    _diffuseR = r;
    _diffuseG = g;
    _diffuseB = b;
}

void Material::setSpecular(float r, float g, float b) {
    _specularR = r;
    _specularG = g;
    _specularB = b;
}

void Material::setSpecularParameters(float intensity, float hardness) {
    _specularIntensity = intensity;
    _specularHardness = hardness;
}

void Material::setAmbient(float r, float g, float b) {
    _ambientR = r;
    _ambientG = g;
    _ambientB = b;
}

void Material::pushMaterial(Context * context) {
    glActiveTexture(GL_TEXTURE0);
    if (this->textures.size() >= 1) {
        glBindTexture(GL_TEXTURE_2D, this->textures[0].getID());
        context->program().setUniform1i("useTextures", 1);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, 0);
        context->program().setUniform1i("useTextures", 0);
    }

    context->program().setUniform3f("material.ambientColor", _ambientR, _ambientG, _ambientB);
    context->program().setUniform3f("material.diffuseColor", _diffuseR, _diffuseG, _diffuseB);
    context->program().setUniform3f("material.specularColor", _specularR, _specularG, _specularB);

    context->program().setUniform1f("material.specularIntensity", _specularIntensity);
    context->program().setUniform1f("material.specularHardness", _specularHardness);

    context->program().setUniform1i("material.emit", _emit);
}