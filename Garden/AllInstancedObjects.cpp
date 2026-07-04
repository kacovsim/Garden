//----------------------------------------------------------------------------------------
/**
 * \file    AllInstancedObjects.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   File with all instanced objects class declarations
 */


#include "allObjects.h"

FlowerInstanced::FlowerInstanced(ShaderProgram* shdrPrg)
    : InstancedModel(shdrPrg, "models/flower00.obj", 0.13f)
{
}

RocksInstanced::RocksInstanced(ShaderProgram* shdrPrg)
    : InstancedModel(shdrPrg, "models/rocks.obj", 0.3f)
{
}

GrassInstanced::GrassInstanced(ShaderProgram* shdrPrg)
    : InstancedModel(shdrPrg, "models/grass.obj", 0.3f)
{
}

BellInstanced::BellInstanced(ShaderProgram* shdrPrg)
    : InstancedModel(shdrPrg, "models/flower01.obj", 0.05f)
{
}

DaisyInstanced::DaisyInstanced(ShaderProgram* shdrPrg)
    : InstancedModel(shdrPrg, "models/flower02.obj", 0.02f)
{
}