#include <iostream>
#include <sstream>
#include <fstream>

#include "mtlloader.h"

std::map<std::string, MaterialDef> LoadMaterialFile(const std::string& filePath)
{
    using namespace std;

    map<string, MaterialDef> mtls;

    // open file
    ifstream file;
    file.open(filePath);
    if (!file.is_open())
        return mtls;

    MaterialDef newMtl;
    string mtlName;

    bool first = true;

    string line;
    while (getline(file, line))
    {
        if (line.size() <= 1)
            continue;

        // get the keyword
        istringstream stream(line);
        string keyword;
        stream >> keyword;

        if (keyword == "newmtl")
        {
            // don't add a material on the first run
            if (first)
                first = false;
            else
            {
                // add material and reset to defaults
                mtls[mtlName] = newMtl;
                newMtl = MaterialDef();
            }

            stream >> mtlName;

        }
        else if (keyword == "Ns")
        {
            stream >> newMtl.specExp;
        }
        else if (keyword == "Ni")
        {
            stream >> newMtl.ior;
        }
        else if (keyword == "Kd")
        {
            stream >> newMtl.diffuseTint[0] >> 
                      newMtl.diffuseTint[1] >> 
                      newMtl.diffuseTint[2];
        }
        else if (keyword == "Ks")
        {
            stream >> newMtl.specTint[0] >> 
                      newMtl.specTint[1] >> 
                      newMtl.specTint[2];
        }
        else if (keyword == "map_Ka")
        {
            stream >> newMtl.metallicMap;
        }
        else if (keyword == "map_Kd")
        {
            stream >> newMtl.diffuseMap;
        }
        else if (keyword == "map_Ns")
        {
            stream >> newMtl.roughnessMap;
        }
        else if (keyword == "map_bump")
        {
            stream >> newMtl.bmpMap;
        }
    }

    file.close();
    return mtls;
}