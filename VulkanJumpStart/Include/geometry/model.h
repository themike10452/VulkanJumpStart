#ifndef GEOM_MODEL_HEADER
#define GEOM_MODEL_HEADER

#include "geometry/TVector3D.h"

#include <fstream>
#include <vector>
#include <string>
#include <sstream>

class Ver
{
public:
    uint32_t Index;
};

class Model
{
public:
    std::vector<Vector3DF> Vertices;
    std::vector<uint32_t> Indices;
};

static void Split( std::string* pText, char delimiter, std::vector<std::string>* pOutParts )
{
    pOutParts->clear();

    std::stringstream ss( *pText );

    std::string part;
    while (std::getline( ss, part, delimiter ))
    {
        pOutParts->push_back( part );
    }
}

static void LoadModel( const char* pFilename, Model* pOutModel )
{
    std::ifstream file( pFilename );

    bool vertices = false;
    bool indices = false;

    int indexOffset = 0;

    std::string line;
    std::vector<std::string> parts;
    while (std::getline( file, line, '\n' ))
    {
        if (!line.compare( "#vertices" ))
        {
            vertices = true;
            indices = false;
            continue;
        }

        if (!line.compare( "#faces" ))
        {
            vertices = false;
            indices = true;
            indexOffset = pOutModel->Indices.size();
            continue;
        }

        if (line[0] == '#')
            continue;

        if (vertices)
        {
            Split( &line, ',', &parts );

            pOutModel->Vertices.push_back( Vector3DF( std::stof( parts[0] ), std::stof( parts[1] ), std::stof( parts[2] ) ) );
        }

        if (indices)
        {
            Split( &line, ',', &parts );

            for ( auto& part : parts )
            {
                pOutModel->Indices.push_back( std::stoi( part ) + indexOffset );
            }
        }
    }

    file.close();
}

#endif // !GEOM_MODEL_HEADER