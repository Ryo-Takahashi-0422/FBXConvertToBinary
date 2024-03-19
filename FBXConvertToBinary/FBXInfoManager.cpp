#include <FBXInfoManager.h>

FBXInfoManager& FBXInfoManager::Instance()
{
    static FBXInfoManager instance;
    return instance;
};

int FBXInfoManager::Init(std::string _modelPath)
{
    FbxManager* manager = nullptr;
    //FbxScene* scene = nullptr;

    modelPath = _modelPath;

    // create manager
    manager = FbxManager::Create();

    // create IOSetting
    FbxIOSettings* ioSettings = FbxIOSettings::Create(manager, IOSROOT);

    // create Importer
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (importer->Initialize(modelPath.c_str(), -1, manager->GetIOSettings()) == false) {

        return -1; // failed
    }

    // Scene�I�u�W�F�N�g��FBX�t�@�C�����̏��𗬂�����
    scene = FbxScene::Create(manager, "scene");
    importer->Import(scene);
    importer->Destroy(); // �V�[���𗬂����񂾂�Importer�͉������OK

    FbxGeometryConverter converter(manager);
    // �|���S�����O�p�`�ɂ���
    //converter.Triangulate(scene, true);

    // FbxGeometryConverter converter(manager);
    // �SMesh����
    converter.SplitMeshesPerMaterial(scene, true);

    // Scene���
    ReadFBXFile();

    auto it = indexWithBonesNumAndWeight.begin();
    int meshIndex = 0;
    int vertexIndex = 0;
    int VertexTotalNum = finalVertexDrawOrder[0].second.vertices.size(); // ���[�v������i�Ƃ̒l��r�ɗp����B���̒l��i�����B������meshIndex���C���N�������g���āA���̃��b�V����ǂށB���̍ۂɂ��̃��b�V����vertex�������Z���Ă����B
    int lastVertexTotalNum = 0;
    for (int i = 0; i < indexWithBonesNumAndWeight.size(); ++i)
    {
        if (i == VertexTotalNum)
        {
            ++meshIndex;
            lastVertexTotalNum = VertexTotalNum;
            VertexTotalNum += finalVertexDrawOrder[meshIndex].second.vertices.size();
            vertexIndex = 0;
        }
        
        auto it2 = it->second.begin();
        for (int j = 0; j < it->second.size(); ++j)
        {
            if (j < 3)
            {
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_index1[j] = it2->first;
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_weight1[j] = it2->second;
            }
            else
            {
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_index2[j - 3] = it2->first;
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_weight2[j - 3] = it2->second;
            }
            ++it2;

        }
        ++it;
        
        ++vertexIndex;
     }

    // �}�l�[�W�����
    // �֘A���邷�ׂẴI�u�W�F�N�g����������
    scene->Destroy();
    manager->Destroy();

    // �t�@�C����������
    const char* fileName = "C:\\Users\\RyoTaka\\Desktop\\InputTest.bin";
    FILE* fp = nullptr;
    fopen_s(&fp, fileName, "wb");
    if (fp == nullptr) {
        //�G���[����
        assert(0);
        return ERROR_FILE_NOT_FOUND;
    }

    // finalVertexInfo�p�f�[�^����
    auto itfinalVertexDrawOrder = finalVertexDrawOrder.begin();
    int finalVertexDrawOrderSize = finalVertexDrawOrder.size();
    fwrite(&finalVertexDrawOrderSize, sizeof(finalVertexDrawOrderSize), 1, fp);

    for (int i = 0; i < finalVertexDrawOrder.size(); ++i)
    {        
        auto meshName = itfinalVertexDrawOrder->first;
        unsigned int meshNameSize = meshName.length();

        fwrite(&meshNameSize, sizeof(meshNameSize), 1, fp);
        fwrite(meshName.c_str(), meshNameSize, 1, fp);

        int verticesSize = itfinalVertexDrawOrder->second.vertices.size();
        fwrite(&verticesSize, sizeof(verticesSize), 1, fp);

        int indicesSize = itfinalVertexDrawOrder->second.indices.size();
        fwrite(&indicesSize, sizeof(indicesSize), 1, fp);

        // FBXVertex ��������
        for (auto& vertex : itfinalVertexDrawOrder->second.vertices)
        {
            fwrite(&vertex, sizeof(vertex), 1, fp);
        }

        // Indeice ��������
        for (auto& indice : itfinalVertexDrawOrder->second.indices)
        {
            fwrite(&indice, sizeof(indice), 1, fp);
        }
        ++itfinalVertexDrawOrder; 
    }

    // finalVertexInfo�p�f�[�^����
    auto itFinalPhongMaterialOrder = finalPhongMaterialOrder.begin();
    int finalPhongMaterialOrderSize = finalPhongMaterialOrder.size();
    fwrite(&finalPhongMaterialOrderSize, sizeof(finalPhongMaterialOrderSize), 1, fp);
    
    for (int i = 0; i < finalPhongMaterialOrder.size(); ++i)
    {
        auto meshName = itFinalPhongMaterialOrder->first;
        unsigned int meshNameSize = meshName.length();

        fwrite(&meshNameSize, sizeof(meshNameSize), 1, fp);
        fwrite(meshName.c_str(), meshNameSize, 1, fp);

        fwrite(&itFinalPhongMaterialOrder->second, sizeof(itFinalPhongMaterialOrder->second), 1, fp);
        ++itFinalPhongMaterialOrder;
    }

    // materialAndTexturenameInfo�p�f�[�^����
    auto itMaterialAndTexturenameInfo = materialAndTexturenameInfo.begin();
    int materialAndTexturenameInfoSize = materialAndTexturenameInfo.size();
    fwrite(&materialAndTexturenameInfoSize, sizeof(materialAndTexturenameInfoSize), 1, fp);
    for (int i = 0; i < materialAndTexturenameInfo.size(); ++i)
    {
        // �}�e���A�����̓ǂݏ���
        auto materialName = itMaterialAndTexturenameInfo->first;
        unsigned int materialNameSize = materialName.length();
        fwrite(&materialNameSize, sizeof(materialNameSize), 1, fp);
        fwrite(materialName.c_str(), materialNameSize, 1, fp);

        // �e�N�X�`���p�X�̓ǂݏ���
        auto texturePath = itMaterialAndTexturenameInfo->second;
        unsigned int texturePathSize = texturePath.length();
        fwrite(&texturePathSize, sizeof(texturePathSize), 1, fp);
        fwrite(texturePath.c_str(), texturePathSize, 1, fp); // �e�N�`���p�X

        ++itMaterialAndTexturenameInfo;
    }

    // localPosAndRotOfMesh�p�̃f�[�^����
    auto itlocalPosAndRotOfMesh = localPosAndRotOfMesh.begin();
    int localPosAndRotOfMeshSize = localPosAndRotOfMesh.size();
    fwrite(&localPosAndRotOfMeshSize, sizeof(localPosAndRotOfMeshSize), 1, fp);
    for (int i = 0; i < localPosAndRotOfMesh.size(); ++i)
    {
        // ���b�V�����̓ǂݏ���
        auto meshName = itlocalPosAndRotOfMesh->first;
        unsigned int meshNameSize = meshName.length();
        fwrite(&meshNameSize, sizeof(meshNameSize), 1, fp);
        fwrite(meshName.c_str(), meshNameSize, 1, fp);

        fwrite(&itlocalPosAndRotOfMesh->second.first, sizeof(itlocalPosAndRotOfMesh->second.first), 1, fp);
        fwrite(&itlocalPosAndRotOfMesh->second.second, sizeof(itlocalPosAndRotOfMesh->second.second), 1, fp);
        ++itlocalPosAndRotOfMesh;
    }

    // animationNameAndBoneNameWithTranslationMatrix�p�̃f�[�^����
    auto itAnimationNameAndBoneNameWithTranslationMatrix = animationNameAndBoneNameWithTranslationMatrix.begin();
    int animationNameAndBoneNameWithTranslationMatrixSize = animationNameAndBoneNameWithTranslationMatrix.size();
    fwrite(&animationNameAndBoneNameWithTranslationMatrixSize, sizeof(animationNameAndBoneNameWithTranslationMatrixSize), 1, fp);
    for (int i = 0; i < animationNameAndBoneNameWithTranslationMatrixSize; ++i)
    {
        // �A�j���[�V�������̓ǂݏ���
        auto animationName = itAnimationNameAndBoneNameWithTranslationMatrix->first;
        unsigned int animationNameSize = animationName.length();
        fwrite(&animationNameSize, sizeof(animationNameSize), 1, fp);
        fwrite(animationName.c_str(), animationNameSize, 1, fp);

        int tempBoneNum = itAnimationNameAndBoneNameWithTranslationMatrix->second.size();
        fwrite(&tempBoneNum, sizeof(tempBoneNum), 1, fp); // �{�[������������

        auto itBoneNumAndMatrix = itAnimationNameAndBoneNameWithTranslationMatrix->second.begin();
        int tempMatrixNum = itBoneNumAndMatrix->second.size();
        fwrite(&tempMatrixNum, sizeof(tempMatrixNum), 1, fp); // �t���[������������
        
        for (int j = 0; j < tempBoneNum; ++j)
        {        
            for (int k = 0; k < tempMatrixNum; ++k)
            {
                XMMATRIX tempMatrix = itBoneNumAndMatrix->second[k];
                fwrite(&tempMatrix, sizeof(tempMatrix), 1, fp);
            }

            ++itBoneNumAndMatrix;
        }

        ++itAnimationNameAndBoneNameWithTranslationMatrix;
    }

    // bonesInitialPostureMatrix�p�̃f�[�^����
    int bonesInitialPostureMatrixSize = bonesInitialPostureMatrix.size();
    fwrite(&bonesInitialPostureMatrixSize, sizeof(bonesInitialPostureMatrixSize), 1, fp);
    for (int i = 0; i < bonesInitialPostureMatrixSize; ++i)
    {
        int boneNum = i;
        fwrite(&boneNum, sizeof(boneNum), 1, fp);

        XMMATRIX tempMatrix;
        tempMatrix = bonesInitialPostureMatrix[i];
        fwrite(&tempMatrix, sizeof(tempMatrix), 1, fp);
    }

    // vertexListOfOBB�p�f�[�^����
    auto itvertexListOfOBB = vertexListOfOBB.begin();
    int vertexListOfOBBSize = vertexListOfOBB.size();
    fwrite(&vertexListOfOBBSize, sizeof(vertexListOfOBBSize), 1, fp);

    for (int i = 0; i < vertexListOfOBBSize; ++i)
    {
        auto meshName = itvertexListOfOBB->first;
        unsigned int meshNameSize = meshName.length();

        fwrite(&meshNameSize, sizeof(meshNameSize), 1, fp);
        fwrite(meshName.c_str(), meshNameSize, 1, fp);

        int verticesSize = itvertexListOfOBB->second.vertices.size();
        fwrite(&verticesSize, sizeof(verticesSize), 1, fp);

        int indicesSize = itvertexListOfOBB->second.indices.size();
        fwrite(&indicesSize, sizeof(indicesSize), 1, fp);

        // FBXVertex ��������
        for (auto& vertex : itvertexListOfOBB->second.vertices)
        {
            fwrite(&vertex, sizeof(vertex), 1, fp);
        }

        // Indeice ��������
        for (auto& indice : itvertexListOfOBB->second.indices)
        {
            fwrite(&indice, sizeof(indice), 1, fp);
        }
        ++itvertexListOfOBB;
    }


    // localPosAndRotOfOBB�p�̃f�[�^����
    auto itlocalPosAndRotOfOBB = localPosAndRotOfOBB.begin();
    int localPosAndRotOfOBBSize = localPosAndRotOfOBB.size();
    fwrite(&localPosAndRotOfOBBSize, sizeof(localPosAndRotOfOBBSize), 1, fp);
    for (int i = 0; i < localPosAndRotOfOBB.size(); ++i)
    {
        // ���b�V�����̓ǂݏ���
        auto meshName = itlocalPosAndRotOfOBB->first;
        unsigned int meshNameSize = meshName.length();
        fwrite(&meshNameSize, sizeof(meshNameSize), 1, fp);
        fwrite(meshName.c_str(), meshNameSize, 1, fp);

        fwrite(&itlocalPosAndRotOfOBB->second.first, sizeof(itlocalPosAndRotOfOBB->second.first), 1, fp);
        fwrite(&itlocalPosAndRotOfOBB->second.second, sizeof(itlocalPosAndRotOfOBB->second.second), 1, fp);
        ++itlocalPosAndRotOfOBB;
    }
    
    std::cout << fileName << "�ɏ������݂܂����B" << std::endl;

    //ofs.close();
    fclose(fp);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���������b�V���͓ǂݍ��߂邪���[�J�����W�ŕ`�悷�邽�ߏd������B�ǂݍ��݃��f�������b�V���������邱�Ƃŉ������B
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FBXInfoManager::ReadFBXFile()
{
    FbxMesh* fbxMesh = nullptr;
    std::vector<int> indiceVec; // �E��n�C���f�N�X
    std::map<std::string, std::vector<int>> fixedIndiceVec; // ����n�C���f�N�X]DirectX�p
    //std::map<std::string, std::vector<LambertInfo>> m_LambertInfo;
    //std::map<std::string, PhongInfo> m_PhongInfo;
    int nameCnt = 0;
    int materialNameCnt = 0;
    int textureNumCnt = 0;

    int meshCount = scene->GetSrcObjectCount<FbxMesh>();

    for(int i = 0; i < meshCount; ++i)
    {
        //Get vertex information(pos, index, uv, normal)
        // ���_x,y,z���W�̒��o�y��vertices�ւ̊i�[
        fbxMesh = scene->GetSrcObject<FbxMesh>(i);
        const char* name = fbxMesh->GetName();
        std::string targetName = name;
        targetName = targetName.substr(0, 3).c_str();

        // UV�Z�b�g���̎擾
        // * 1��UV�Z�b�g���̂ݑΉ�
        FbxStringList uvSetNameList;
        fbxMesh->GetUVSetNames(uvSetNameList);
        const char* uvSetName = uvSetNameList.GetStringAt(0);

        // OBB�̏���
        int indiceIndexOfOBB = 0;
        FbxDouble3 localTransition; // ���f���̃��[�J��X,Y,Z���W(blender�œK�p�����ꍇ�͎擾�s�\)
        FbxDouble3 localRotation; // ���f���̃��[�J��X,Y,Z��]�p�x(blender�Ŋp�x�K�p�����ꍇ�͎擾�s�\)
        if (targetName == "OBB")
        {
            // ���_���̏���
            int controlPointCount = fbxMesh->GetControlPointsCount();
            VertexInfo vertex;
            for (int i = 0; i < controlPointCount; i++)
            {
                // ���_���W��ǂݍ���Őݒ�
                auto point = fbxMesh->GetControlPointAt(i);                
                vertex.vertices.resize(controlPointCount);
                vertex.vertices[i].pos.x = point[0];
                vertex.vertices[i].pos.y = point[1];
                vertex.vertices[i].pos.z = point[2];
            }
            std::pair<std::string, VertexInfo> vertexInfo;
            vertexInfo.first = name;
            vertexInfo.second = vertex;
            

            // �C���e�b�N�X���̏���
            std::vector<unsigned int> indices;
            std::vector<std::array<int, 2>> oldNewIndexPairList;

            
            //for (int polIndex = 0; polIndex < fbxMesh->GetPolygonCount(); polIndex++) // �|���S�����̃��[�v
            //{
            //    for (int polVertexIndex = 0; polVertexIndex < fbxMesh->GetPolygonSize(polIndex); polVertexIndex++) // ���_���̃��[�v
            //    {
            //        // �C���f�b�N�X���W
            //        auto vertexIndex = fbxMesh->GetPolygonVertex(polIndex, polVertexIndex);
            //        vertexIndex += indiceIndexOfOBB;

            //        // �C���f�b�N�X���W��ݒ�B�������ꂽ���b�V���̃C���f�b�N�X�́A�������Ȃ��Ɣԍ���0����U�蒼�����B����A�C���f�b�N�X�̓��b�V������������Ă��悤���P��̂��̂��낤���ʂ��ԍ��Ȃ̂ŁA
            //        // ���b�V���𕪊�����ꍇ�͈�O�ɓǂݍ��񂾃��b�V���̃C���f�b�N�X�ԍ��̓��A�u�ő�̒l + 1�v�������̂�ǉ�����K�v������B
            //        indices.push_back(vertexIndex);
            //    }
            //}
            // blender����̏o�͌��ʂ��������Ȃ�(�����̓����Ƀ|���S�����`�悳���C���f�N�X���o�͂���Ă���)���߁A����͂őΉ�����B
            // �΂ߐ������ꂽ���̂ŁA�ʂ��΂߂Ɍ��ԃC���f�N�X�ɂ��Ă���
            indices.push_back(0);
            indices.push_back(1);
            indices.push_back(2);

            indices.push_back(2);
            indices.push_back(0);
            indices.push_back(3);

            indices.push_back(3);
            indices.push_back(2);
            indices.push_back(6);

            indices.push_back(6);
            indices.push_back(3);
            indices.push_back(7);

            indices.push_back(7);
            indices.push_back(6);
            indices.push_back(4);

            indices.push_back(4);
            indices.push_back(6);
            indices.push_back(5);

            indices.push_back(5);
            indices.push_back(6);
            indices.push_back(2);

            indices.push_back(2);
            indices.push_back(5);
            indices.push_back(1);

            indices.push_back(5);
            indices.push_back(1);
            indices.push_back(0);

            indices.push_back(0);
            indices.push_back(5);
            indices.push_back(4);

            indices.push_back(4);
            indices.push_back(7);
            indices.push_back(3);

            indices.push_back(3);
            indices.push_back(4);
            indices.push_back(0);

            vertexInfo.second.indices = indices;
            vertexListOfOBB.push_back(vertexInfo);
            indiceIndexOfOBB = indices.size();

            // �}�e���A����񌳂̃m�[�h�擾
            FbxNode* node = fbxMesh->GetNode();
            if (node == 0)
            {
                continue;
            }
            // OBB�̂��߃��[�J�����W�E�p�x�擾
            localTransition = node->LclTranslation.Get();
            XMFLOAT3 lPos;
            lPos.x = (float)localTransition.mData[0];
            lPos.y = (float)localTransition.mData[1];
            lPos.z = (float)localTransition.mData[2];
            localPosAndRotOfOBB[name].first = lPos;
            localRotation = node->LclRotation.Get();
            XMFLOAT3 lRot;
            lRot.x = (float)localRotation.mData[0];
            lRot.y = (float)localRotation.mData[1];
            lRot.z = (float)localRotation.mData[2];
            localPosAndRotOfOBB[name].second = lRot;

            continue;
        }

        // ���_���W���̃��X�g�𐶐�
        for (int i = 0; i < fbxMesh->GetControlPointsCount(); i++)
        {
            // ���_���W��ǂݍ���Őݒ�
            auto point = fbxMesh->GetControlPointAt(i);
            std::vector<float> vertex;
            vertex.push_back(point[0]);
            vertex.push_back(point[1]);
            vertex.push_back(point[2]);
            vertexInfoList.push_back(vertex);
        }
        
        // ���_���̏����擾����
        std::vector<unsigned int> indices;
        //std::vector<unsigned int> indicesFiexed4DirectX;
        std::vector<std::array<int, 2>> oldNewIndexPairList;
        for (int polIndex = 0; polIndex < fbxMesh->GetPolygonCount(); polIndex++) // �|���S�����̃��[�v
        {
            for (int polVertexIndex = 0; polVertexIndex < fbxMesh->GetPolygonSize(polIndex); polVertexIndex++) // ���_���̃��[�v
            {
                // �C���f�b�N�X���W
                auto vertexIndex = fbxMesh->GetPolygonVertex(polIndex, polVertexIndex);
                vertexIndex += meshVertIndexStart;

                // ���_���W
                std::vector<float> vertexInfo = vertexInfoList[vertexIndex];

                // �@�����W
                FbxVector4 normalVec4;
                fbxMesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);  

                // UV���W
                FbxVector2 uvVec2;
                bool isUnMapped;
                fbxMesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);
                // �C���f�b�N�X���W�̃`�F�b�N�ƍč̔�
                if (!IsExistNormalUVInfo(vertexInfo))
                {
                    // �@�����W��UV���W�����ݒ�̏ꍇ�A���_���ɕt�^���čĐݒ�
                    vertexInfoList[vertexIndex] = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
                }
                else if (!IsSetNormalUV(vertexInfo, normalVec4, uvVec2))
                {
                    // �����꒸�_�C���f�b�N�X�̒��Ŗ@�����W��UV���W���قȂ�ꍇ�A
                    // �V���Ȓ��_�C���f�b�N�X�Ƃ��č쐬����
                    vertexIndex = CreateNewVertexIndex(vertexInfo, normalVec4, uvVec2, vertexInfoList, vertexIndex, oldNewIndexPairList);
                }

                // �C���f�b�N�X���W��ݒ�B�������ꂽ���b�V���̃C���f�b�N�X�́A�������Ȃ��Ɣԍ���0����U�蒼�����B����A�C���f�b�N�X�̓��b�V������������Ă��悤���P��̂��̂��낤���ʂ��ԍ��Ȃ̂ŁA
                // ���b�V���𕪊�����ꍇ�͈�O�ɓǂݍ��񂾃��b�V���̃C���f�b�N�X�ԍ��̓��A�u�ő�̒l + 1�v�������̂�ǉ�����K�v������B
                indices.push_back(vertexIndex);
            }                
        }

        // ���_���𐶐�
        std::vector<FBXVertex> vertices;
        for (int i = meshVertIndexStart; i < vertexInfoList.size(); i++)
        {
            std::vector<float> vertexInfo = vertexInfoList[i];
            if (vertexInfo.size() > 3)
            {
                // uv�ɂ��āA�^�C�����O������̂�1�ȏ�A0�����̒l������B�������s�N�Z���V�F�[�_�[�ɓn���O�ɂ����������͒��_�V�F�[�_�[�ŉ��H����ƁA�s�N�Z���V�F�[�_�[�ł̏����ɗl�X�ȕs�����������
                vertices.push_back(FBXVertex
                {
                    {
                        vertexInfo[0], vertexInfo[1], vertexInfo[2]
                    },
                    {
                        vertexInfo[3], vertexInfo[4], vertexInfo[5]
                    },
                    {
                        vertexInfo[6], 1.0f - vertexInfo[7] // Blender����o�͂����ꍇ�AV�l�͔��]������(���f�����ŏ��ɍ쐬���ꂽ�Ƃ��̃\�t�g(��FMaya)�͊֌W�Ȃ�)
                    }
                });
            }
        }

        //// �O��ǂݍ��񂾃��b�V���̃C���f�b�N�X�ԍ��̓��A�ő�l�𒊏o����
        //auto iter = std::max_element(indices.begin(), indices.end());
        //size_t index = std::distance(indices.begin(), iter);
        //meshVertIndexStart = indices[index] + 1;

        //// ����n�C���f�N�X�ɏC��
        //for (int i = 0; i < /*fbxMesh->GetPolygonCount()*/indices.size() / 3; ++i)
        //{
        //    // 2 => 1 => 0�ɂ��Ă�͍̂���n�΍�
        //    indicesFiexed4DirectX.push_back(indices[i * 3 + 2]);
        //    indicesFiexed4DirectX.push_back(indices[i * 3 + 1]);
        //    indicesFiexed4DirectX.push_back(indices[i * 3]);
        //}

        char newName[32];
            
        // �}�e���A�����ɕ��������̂ɍ��킹�ă��l�[��
        sprintf_s(newName, "%s%d", name, nameCnt);
        name = newName;
            
        materialNameAndVertexInfo[name] = { vertices, indices/*indicesFiexed4DirectX*/ };
        auto itFI = materialNameAndVertexInfo.begin();
        for (int i = 0; i <= nameCnt; i++)
        {
            ++itFI;
        }
            
        // ���b�V����ǂݍ��񂾏��Ԃɐ��񂷂�B
        // �����������b�V���̒��_��`�悷��Ƃ��A�ʂ̃��b�V���̃C���f�b�N�X�����荞�ނƕ`�悪���s����B
        // �܂��A�X�e�[�W�Ȃǃ��b�V�����������݂���ꍇ�͓��ꂵ�Ă����B���ꂵ�Ȃ��Ƃ��ꂼ��̃��b�V���̃��[�J�����W�ŕ`�悳��邽�߁A���b�V�������b�v������Ԃł̕`��ɂȂ��Ă��܂��B
        // Align meshes in the order in which they are read.
        // When drawing the vertices of a divided mesh, if the index of another mesh interrupts the drawing, the drawing will fail.
        // Also, if there are multiple meshes, such as stages, they should be unified.Otherwise, the local coordinates of each mesh will be used for drawing, resulting in wrapped meshes.
        finalVertexDrawOrder.resize(nameCnt + 1);
        finalVertexDrawOrder.at(nameCnt).first = name;
        finalVertexDrawOrder.at(nameCnt).second.vertices = vertices;
        finalVertexDrawOrder.at(nameCnt).second.indices = indices;

        // �ڋ�ԏ���
        int iTangentCnt = fbxMesh->GetElementTangentCount();
        if (iTangentCnt != 0)
        {
            for (int i = 0; i < finalVertexDrawOrder.at(nameCnt).second.indices.size(); ++i)
            {
                ProcessTangent(fbxMesh, name, nameCnt, finalVertexDrawOrder.at(nameCnt).second.indices[i] - meshVertIndexStart);
                ++testCnt;
            }
        }
        testCnt = 0;

        // �O��ǂݍ��񂾃��b�V���̃C���f�b�N�X�ԍ��̓��A�ő�l�𒊏o����
        auto iter = std::max_element(indices.begin(), indices.end());
        size_t index = std::distance(indices.begin(), iter);
        meshVertIndexStart = indices[index] + 1;

        // Get material information
        // �}�e���A����񌳂̃m�[�h�擾
        FbxNode* node = fbxMesh->GetNode();
        if (node == 0) 
        {
            continue;
        }

        // OBB�̂��߃��[�J�����W�E�p�x�擾
        localTransition = node->LclTranslation.Get();
        XMFLOAT3 lPos;
        lPos.x = (float)localTransition.mData[0];
        lPos.y = (float)localTransition.mData[1];
        lPos.z = (float)localTransition.mData[2];
        localPosAndRotOfMesh[name].first = lPos;
        localRotation = node->LclRotation.Get();
        XMFLOAT3 lRot;
        lRot.x = (float)localRotation.mData[0];
        lRot.y = (float)localRotation.mData[1];
        lRot.z = (float)localRotation.mData[2];
        localPosAndRotOfMesh[name].second = lRot;


        // �}�e���A���̐����`�F�b�N
        int materialNum = node->GetMaterialCount();
        if (materialNum == 0) 
        {
            continue;
        }

        // �}�e���A�������擾
        FbxSurfaceMaterial* material;
        if (modelPath == "C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBX\\NewConnan_ZDir.fbx")
        {
            material = node->GetMaterial(i); // �������������b�V���ł́ui�v�A���Ƀf�o�b�O�p��modelpath == ...�̂悤�ɂ��Ă���(�R�i���̃e�N�X�`���`�抎��battlefield�`��(�^����)�ɍ��킹��)
        }
        else
        {
            material = node->GetMaterial(0);
        }

        if (material != 0) 
        {
            std::string type;
            type = material->GetClassId().GetName();

            // �}�e���A�����:Lambert��Phong��
            if (type == "FbxSurfaceLambert") {
                assert(!"invalid material.");
                exit(1);

                //// ��Lambert pattern process has no implemention
                //// Lambert�Ƀ_�E���L���X�g
                //FbxSurfaceLambert* lambert = (FbxSurfaceLambert*)material;

                //// Diffuse
                //m_LambertInfo[name][i].diffuse[0] = (float)lambert->Diffuse.Get().mData[0];
                //m_LambertInfo[name][i].diffuse[1] = (float)lambert->Diffuse.Get().mData[1];
                //m_LambertInfo[name][i].diffuse[2] = (float)lambert->Diffuse.Get().mData[2];

                //// Ambient
                //m_LambertInfo[name][i].ambient[0] = (float)lambert->Ambient.Get().mData[0];
                //m_LambertInfo[name][i].ambient[1] = (float)lambert->Ambient.Get().mData[1];
                //m_LambertInfo[name][i].ambient[2] = (float)lambert->Ambient.Get().mData[2];

                //// emissive
                //m_LambertInfo[name][i].emissive[0] = (float)lambert->Emissive.Get().mData[0];
                //m_LambertInfo[name][i].emissive[1] = (float)lambert->Emissive.Get().mData[1];
                //m_LambertInfo[name][i].emissive[2] = (float)lambert->Emissive.Get().mData[2];

                //// bump
                //m_LambertInfo[name][i].bump[0] = (float)lambert->Bump.Get().mData[0];
                //m_LambertInfo[name][i].bump[1] = (float)lambert->Bump.Get().mData[1];
                //m_LambertInfo[name][i].bump[2] = (float)lambert->Bump.Get().mData[2];
            }
            else if (type == "FbxSurfacePhong") {

                // Phong�Ƀ_�E���L���X�g
                FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

                // copy and order material data
                finalPhongMaterialOrder.resize(nameCnt + 1);
                finalPhongMaterialOrder.at(nameCnt).first = name;

                // Diffuse
                finalPhongMaterialOrder.at(nameCnt).second.diffuse[0] = (float)phong->Diffuse.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.diffuse[1] = (float)phong->Diffuse.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.diffuse[2] = (float)phong->Diffuse.Get().mData[2];

                // Ambient
                finalPhongMaterialOrder.at(nameCnt).second.ambient[0] = (float)phong->Ambient.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.ambient[1] = (float)phong->Ambient.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.ambient[2] = (float)phong->Ambient.Get().mData[2];

                // emissive
                finalPhongMaterialOrder.at(nameCnt).second.emissive[0] = (float)phong->Emissive.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.emissive[1] = (float)phong->Emissive.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.emissive[2] = (float)phong->Emissive.Get().mData[2];

                // bump
                finalPhongMaterialOrder.at(nameCnt).second.bump[0] = (float)phong->Bump.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.bump[1] = (float)phong->Bump.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.bump[2] = (float)phong->Bump.Get().mData[2];

                // specular
                finalPhongMaterialOrder.at(nameCnt).second.specular[0] = (float)phong->Specular.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.specular[1] = (float)phong->Specular.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.specular[2] = (float)phong->Specular.Get().mData[2];

                // reflectivity
                finalPhongMaterialOrder.at(nameCnt).second.reflection[0] = (float)phong->Reflection.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.reflection[1] = (float)phong->Reflection.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.reflection[2] = (float)phong->Reflection.Get().mData[2];

                // shiness
                finalPhongMaterialOrder.at(nameCnt).second.transparency = (float)phong->TransparencyFactor.Get();
            }
        }


        // Get texture infomation
        for (auto type : textureType)
        {
            // �f�B�t���[�Y�v���p�e�B������
            FbxProperty property = material->FindProperty(/*FbxSurfaceMaterial::sDiffuse*/type);

            // �v���p�e�B�������Ă��郌�C���[�h�e�N�X�`���̖������`�F�b�N
            int layerNum = property.GetSrcObjectCount<FbxLayeredTexture>();

            // ���C���[�h�e�N�X�`����������Βʏ�e�N�X�`��
            if (layerNum == 0) {
                // �ʏ�e�N�X�`���̖������`�F�b�N
                int numGeneralTexture = property.GetSrcObjectCount<FbxFileTexture>();

                // �e�e�N�X�`���ɂ��ăe�N�X�`�������Q�b�g
                for (int i = 0; i < numGeneralTexture; ++i) {
                    // i�Ԗڂ̃e�N�X�`���I�u�W�F�N�g�擾
                    FbxFileTexture* texture = /*FbxCast<FbxFileTexture>*/property.GetSrcObject<FbxFileTexture>(i);

                    // �e�N�X�`���t�@�C���p�X���擾�i�t���p�X�j
                    const char* filePath = texture->GetFileName();

                    materialAndTexturenameInfo.resize(textureNumCnt);
                    std::pair<std::string, std::string> pair = { name, filePath };
                    materialAndTexturenameInfo.push_back(pair);
                    ++textureNumCnt;

                }
            }
        }


        // Get bone pos and animation matrix
        // �X�L���̐����擾
        int skinCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);

        // No skin model can't process and occur error
        if (skinCount > 0) 
        {
            // �A�j���[�V���������擾
            FbxGlobalSettings& globalSettings = scene->GetGlobalSettings();
            FbxTime::EMode timeMode = globalSettings.GetTimeMode();
            FbxTime period;
            period.SetTime(0, 0, 0, 1, 0, timeMode);

            for (int skinNum = 0; skinNum < skinCount; ++skinNum) {
                // skinNum�Ԗڂ̃X�L�����擾
                FbxStatus* eStatus;
                FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(skinNum, FbxDeformer::eSkin));

                int cluster_count = skin->GetClusterCount();
                lastMeshIndexNumByCluster = indexWithBonesNumAndWeight.size();
                for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
                {
                    // get informations per bone(cluster index point to bone)
                    FbxCluster* cluster = skin->GetCluster(cluster_index); // bone like "Head", "Neck", etc...
                    int pointNum = cluster->GetControlPointIndicesCount();
                    int* pointAry = cluster->GetControlPointIndices();
                    double* weightAry = cluster->GetControlPointWeights();

                    for (int i = 0; i < pointNum; ++i) {
                        // ���_�C���f�b�N�X�ƃE�F�C�g���擾
                        int index = pointAry[i]; //////////////////////////
                        index += lastMeshIndexNumByCluster;
                        float weight = (float)weightAry[i];

                        indexWithBonesNumAndWeight[index][cluster_index] = weight; // vertex index(serial number=�ʂ��ԍ�), bone index, bone weight set
                    }                                        
                    
                    // �{�[���̏����p�����擾
                    if (!isBonesInitialPostureMatrixFilled)
                    {
                        fbxsdk::FbxAMatrix init_matrix;
                        cluster->GetTransformLinkMatrix(init_matrix); // �����p��
                        XMVECTOR v0 = { init_matrix[0].mData[0] ,init_matrix[0].mData[1] ,init_matrix[0].mData[2] ,init_matrix[0].mData[3] };
                        XMVECTOR v1 = { init_matrix[1].mData[0] ,init_matrix[1].mData[1] ,init_matrix[1].mData[2] ,init_matrix[1].mData[3] };
                        XMVECTOR v2 = { init_matrix[2].mData[0] ,init_matrix[2].mData[1] ,init_matrix[2].mData[2] ,init_matrix[2].mData[3] };
                        XMVECTOR v3 = { init_matrix[3].mData[0] ,init_matrix[3].mData[1] ,init_matrix[3].mData[2] ,init_matrix[3].mData[3] };
                        bonesInitialPostureMatrix[cluster_index].r[0] = v0;
                        bonesInitialPostureMatrix[cluster_index].r[1] = v1;
                        bonesInitialPostureMatrix[cluster_index].r[2] = v2;
                        bonesInitialPostureMatrix[cluster_index].r[3] = v3;


                        FbxNode* linked_node = cluster->GetLink();
                        FbxArray< FbxString* > takeNameAry;   // ������i�[�z��
                        scene->FillAnimStackNameArray/*FillTakeNameArray*/(takeNameAry);  // �e�C�N���擾
                        int numTake = takeNameAry.GetCount();     // �e�C�N��
                        FbxTime start;
                        FbxTime stop;

                        // "Take" is Skin name and Animation name set like "Armature|Walking", "Armature|Punching", etc.
                        for (int i = 0; i < numTake; ++i)
                        {
                            // �����A�j���[�V�������؂�ւ�
                            FbxAnimStack* pStack = scene->GetSrcObject<FbxAnimStack>(i);
                            scene->SetCurrentAnimationStack(pStack);

                            // �e�C�N������e�C�N�����擾
                            FbxTakeInfo* currentTakeInfo = scene->GetTakeInfo(*(takeNameAry[i]));
                            if (currentTakeInfo)
                            {
                                start = currentTakeInfo->mLocalTimeSpan.GetStart();
                                stop = currentTakeInfo->mLocalTimeSpan.GetStop();

                                // 1�t���[�����ԁiperiod�j�Ŋ���΃t���[�����ɂȂ�
                                int startFrame = (int)(start.Get() / period.Get());
                                int stopFrame = (int)(stop.Get() / period.Get());
                                for (int j = startFrame; j < stopFrame; ++j)
                                {
                                    FbxMatrix mat;
                                    FbxTime time = start + period * j;
                                    mat = scene->GetAnimationEvaluator()->GetNodeGlobalTransform(linked_node, time);

                                    XMVECTOR v0 = { mat[0].mData[0] ,mat[0].mData[1] ,mat[0].mData[2] ,mat[0].mData[3] };
                                    XMVECTOR v1 = { mat[1].mData[0] ,mat[1].mData[1] ,mat[1].mData[2] ,mat[1].mData[3] };
                                    XMVECTOR v2 = { mat[2].mData[0] ,mat[2].mData[1] ,mat[2].mData[2] ,mat[2].mData[3] };
                                    XMVECTOR v3 = { mat[3].mData[0] ,mat[3].mData[1] ,mat[3].mData[2] ,mat[3].mData[3] };

                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[0] = v0;
                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[1] = v1;
                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[2] = v2;
                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[3] = v3;
                                }
                            }
                        }
                    }
                }
                isBonesInitialPostureMatrixFilled = true;
            }

            // �O����(���_�C���f�b�N�X�ƃE�F�C�g���擾)�ɂ���Ēǉ����ꂽ���_�C���f�b�N�X�̓N���X�^�[����͓ǂݎ��Ȃ����߁A�ǉ����ꂽ�C���f�b�N�X�ƌ��ƂȂ�C���f�b�N�X�̃}�b�v����e�����󂯂�{�[���ԍ�����т��̃E�F�C�g�A�ڋ�ԏ����R�s�[���Ă����B
            auto itAddtionalIndex = addtionalVertexIndexByApplication.begin();
            for (int i = 0; i < addtionalVertexIndexByApplication.size(); ++i)
            {
                for (int j = 0; j < itAddtionalIndex->second.size(); ++j)
                {
                    auto iter = indexWithBonesNumAndWeight.find(itAddtionalIndex->first);
                    if (iter != indexWithBonesNumAndWeight.end())
                    {
                        indexWithBonesNumAndWeight[itAddtionalIndex->second[j]] = iter->second;
                    }
                }
                ++itAddtionalIndex;
            }
        }

        // �������b�V���̃}�e���A�����͑S�ē���ƂȂ菈���s�\�ƂȂ邽�߁A���̖����ɐ��l��ǉ����Ă���
        name = node->GetName();
        nameCnt += 1;
    }
}

// �@���AUV��񂪑��݂��Ă��邩�H
bool FBXInfoManager::IsExistNormalUVInfo(const std::vector<float>& vertexInfo)
{
    return vertexInfo.size() == 8; // ���_3 + �@��3 + UV2
}
// ���_���𐶐�
std::vector<float> FBXInfoManager::CreateVertexInfo(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    std::vector<float> newVertexInfo;
    // �ʒu���W
    newVertexInfo.push_back(vertexInfo[0]);
    newVertexInfo.push_back(vertexInfo[1]);
    newVertexInfo.push_back(vertexInfo[2]);
    // �@�����W
    newVertexInfo.push_back(normalVec4[0]);
    newVertexInfo.push_back(normalVec4[1]);
    newVertexInfo.push_back(normalVec4[2]);
    // UV���W
    newVertexInfo.push_back(uvVec2[0]);
    newVertexInfo.push_back(uvVec2[1]);
    return newVertexInfo;
}

// �V���Ȓ��_�C���f�b�N�X�𐶐�����
int FBXInfoManager::CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
    std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList)
{
    // �쐬�ς̏ꍇ�A�Y���̃C���f�b�N�X��Ԃ�
    for (int i = 0; i < oldNewIndexPairList.size(); i++)
    {
        int newIndex = oldNewIndexPairList[i][1];
        if (oldIndex == oldNewIndexPairList[i][0]
            && IsSetNormalUV(vertexInfoList[newIndex], normalVec4, uvVec2))
        {
            return newIndex;
        }
    }
    // �쐬�ςłȂ��ꍇ�A�V���Ȓ��_�C���f�b�N�X�Ƃ��č쐬
    std::vector<float> newVertexInfo = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
    vertexInfoList.push_back(newVertexInfo);
    // �쐬�����C���f�b�N�X����ݒ�
    int newIndex = vertexInfoList.size() - 1;
    std::array<int, 2> oldNewIndexPair{ oldIndex , newIndex };
    oldNewIndexPairList.push_back(oldNewIndexPair);

    int reserveNewIndex = newIndex;
    addtionalVertexIndexByApplication[oldIndex].push_back(reserveNewIndex);
    return newIndex;
}
// vertexInfo�ɖ@���AUV���W���ݒ�ς��ǂ���
bool FBXInfoManager::IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    if (fabs(vertexInfo[3] - normalVec4[0]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[6] - uvVec2[0]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[4] - normalVec4[1]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[7] - uvVec2[1]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[5] - normalVec4[2]) > FLT_EPSILON) return false;

    return true;
}

void FBXInfoManager::ProcessTangent(FbxMesh* mesh, std::string materialName, int nameCnt, int indexNum)
{
    if (mesh->GetElementTangentCount() == 0)
    {
        return;
    }

    FbxGeometryElementTangent* tangent = mesh->GetElementTangent(0);
    FbxGeometryElementBinormal* biNormal = mesh->GetElementBinormal(0);
    FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);

    switch (tangent->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
        switch (tangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            //const auto vt = tangent->GetDirectArray().GetAt(vertexIndex);
            //outTangent.x = static_cast<float>(vt.mData[0]);
            //outTangent.y = static_cast<float>(vt.mData[1]);
            //outTangent.z = static_cast<float>(vt.mData[2]);
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>( vt.mData[ 3 ] );
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            //const int index = tangent->GetIndexArray().GetAt(vertexIndex);
            //const auto vt = tangent->GetDirectArray().GetAt(index);
            //outTangent.x = static_cast<float>(vt.mData[0]);
            //outTangent.y = static_cast<float>(vt.mData[1]);
            //outTangent.z = static_cast<float>(vt.mData[2]);
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>( vt.mData[ 3 ] );
        }
        break;

        default:
            assert(!"invalid reference.");
            exit(1);
        }
        break;

    case FbxGeometryElement::eByPolygonVertex:
        switch (tangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt].resize(9);
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][0] = (static_cast<float>(tangent->GetDirectArray().GetAt(testCnt).mData[0]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][1] = (static_cast<float>(tangent->GetDirectArray().GetAt(testCnt).mData[1]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][2] = (static_cast<float>(tangent->GetDirectArray().GetAt(testCnt).mData[2]));

            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][3] = (static_cast<float>(biNormal->GetDirectArray().GetAt(testCnt).mData[0]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][4] = (static_cast<float>(biNormal->GetDirectArray().GetAt(testCnt).mData[1]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][5] = (static_cast<float>(biNormal->GetDirectArray().GetAt(testCnt).mData[2]));

            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][6] = (static_cast<float>(vertexNormal->GetDirectArray().GetAt(testCnt).mData[0]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][7] = (static_cast<float>(vertexNormal->GetDirectArray().GetAt(testCnt).mData[1]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][8] = (static_cast<float>(vertexNormal->GetDirectArray().GetAt(testCnt).mData[2]));
            

            if (std::find(indexOFTangentBinormalNormalByMaterialName[materialName].begin(), indexOFTangentBinormalNormalByMaterialName[materialName].end(), indexNum) == indexOFTangentBinormalNormalByMaterialName[materialName].end())
            {
                indexOFTangentBinormalNormalByMaterialName[materialName].emplace_back(indexNum);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).tangent[0] = static_cast<float>(tangent->GetDirectArray().GetAt(indexNum).mData[0]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).tangent[1] = static_cast<float>(tangent->GetDirectArray().GetAt(indexNum).mData[1]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).tangent[2] = static_cast<float>(tangent->GetDirectArray().GetAt(indexNum).mData[2]);

                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).biNormal[0] = static_cast<float>(biNormal->GetDirectArray().GetAt(indexNum).mData[0]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).biNormal[1] = static_cast<float>(biNormal->GetDirectArray().GetAt(indexNum).mData[1]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).biNormal[2] = static_cast<float>(biNormal->GetDirectArray().GetAt(indexNum).mData[2]);

                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).vNormal[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(indexNum).mData[0]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).vNormal[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(indexNum).mData[1]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).vNormal[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(indexNum).mData[2]);
            }
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt( vertexCounter ).mData[ 3 ]);            
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            //int index = tangent->GetIndexArray().GetAt(vertexCounter);
            //outTangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[0]);
            //outTangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[1]);
            //outTangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[2]);
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt( index ).mData[ 3 ]);
        }
        break;

        default:
            assert(!"Invalid reference for tangent.");
            exit(1);
        }
        break;

    case FbxGeometryElement::eNone:
    case FbxGeometryElement::eByPolygon:
    case FbxGeometryElement::eByEdge:
    case FbxGeometryElement::eAllSame:
        std::cerr << "Unhandled mapping mode for tangent.";
        exit(1);
        break;
    }
}