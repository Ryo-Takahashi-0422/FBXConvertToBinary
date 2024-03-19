#include <FBXInfoManager.h>

int main() {
	std::vector< std::string> modelPath;
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\BattleField_Test.fbx");
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\Boxs_diagonal.fbx"); 
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\Box_diagonal_lpos.fbx"); 
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\Box_diagonal.fbx"); 
	// 3 texture model
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\ancient\\ziggurat_test2.fbx");
	modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\sponza3.fbx");

	// 4 textures model
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\NewConnan\\NewConnan3.fbx");
	//modelPath.push_back("C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBX\\NewConnan_ZDir.fbx");

	auto fbxInfoManager = FBXInfoManager::Instance();
	fbxInfoManager.Init(modelPath[0]);
	return 0;
}