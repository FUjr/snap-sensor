#include "LoadModel.cc"

LoadModel::LoadModel(String filename)
{
    SPIFFS.begin();
    File file = SPIFFS.open(filename, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");

    }
    this->model_size = file.size();
    this->model = (char *)malloc(this->model_size);
    if (model == nullptr)
    {
        Serial.println("Failed to allocate memory for model");
    }
    file.readBytes(model, file.size());
    file.close();
}

LoadModel::~LoadModel()
{
    free(this->model);
}
