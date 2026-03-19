import unreal
import csv

def override_vertices():
    # 配置路径
    csv_path = "/Game/CustomData/vertices.csv"
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    # 读取CSV
    with open(unreal.SystemLibrary.convert_to_absolute_path(csv_path), 'r') as f:
        reader = csv.DictReader(f)
        mesh_data = {}
        for row in reader:
            mesh_name = row['MeshName']
            if mesh_name not in mesh_data:
                mesh_data[mesh_name] = []
            mesh_data[mesh_name].append((
                int(row['VertexIndex']),
                unreal.Vector(
                    float(row['X']), 
                    float(row['Z']),  # UE坐标系Y→Z转换
                    float(row['Y'])
                )
            ))
    
    # 修改网格体
    for mesh_name, vertices in mesh_data.items():
        # 加载静态网格
        mesh = unreal.load_asset(f"/Game/Architecture/{mesh_name}.{mesh_name}")
        if not mesh:
            print(f"Error: {mesh_name} not found!")
            continue
            
        # 获取LOD0顶点数据
        lod0 = mesh.get_editor_property('static_materials')[0].lod_data
        positions = lod0.vertex_positions
        
        # 覆盖顶点坐标
        for (index, new_pos) in vertices:
            if index < len(positions):
                positions[index] = new_pos
            else:
                print(f"VertexIndex {index} out of range for {mesh_name}")
        
        # 提交修改并保存
        mesh.set_editor_property('static_materials', [lod0])
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)