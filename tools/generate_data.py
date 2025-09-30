import json
import pandas as pd
import numpy as np
import os
import glob
from PIL import Image, ImageDraw

# SAMPLE_SPELLS, SAMPLE_SCROLL_SPELLS, SAMPLE_SPELL_CHARGES (previous, omitted for brevity)

def simple_perlin(width, height, seed=42, octaves=4, persistence=0.5):
    np.random.seed(seed)
    noise = np.random.random((width, height))
    for o in range(1, octaves):
        scale = 2 ** o
        small_noise = np.random.random((width // scale + 1, height // scale + 1)) * persistence
        noise += np.repeat(np.repeat(small_noise, scale, axis=0), scale, axis=1)[:width, :height]
    return (noise - noise.min()) / (noise.max() - noise.min())

def get_biome(elev, moist):
    if elev > 0.7: return "mountain"
    if moist > 0.7: return "swamp"
    if moist < 0.3 and elev < 0.3: return "desert"
    if moist > 0.5: return "forest"
    return "plains"

def get_biome_tiles(biome):
    tiles = {
        "forest": ["grass_wispy", "tree_oak"],
        "plains": ["grass_tuft"],
        "desert": ["sand_dune"],
        "swamp": ["water_wave", "grass_tuft"],
        "mountain": ["rock_boulder"]
    }
    return tiles.get(biome, ["grass_tuft"])

def generate_tile_atlas(tile_id, frames=4, size=64):
    atlas = Image.new('RGBA', (size * frames, size), (0,0,0,0))
    for f in range(frames):
        if tile_id == 'water_wave':
            img = Image.new('RGBA', (size, size), (64,164,223,128))
            draw = ImageDraw.Draw(img)
            for x in range(size):
                y_wave = int(32 + 10 * np.sin(x * 0.1 + f * 0.5))
                draw.line([(x, y_wave-5), (x, y_wave+5)], fill=(255,255,255,100), width=2)
        elif tile_id == 'grass_wispy':
            img = Image.new('RGBA', (size, size), (34,139,34,255))
            draw = ImageDraw.Draw(img)
            for _ in range(np.random.randint(5,15)):
                bx = np.random.randint(0,size)
                by = size
                length = np.random.randint(10,20)
                draw.line([(bx, by), (bx + np.random.randint(-2,2), by - length)], fill=(0,100,0,255), width=1)
        else:
            img = Image.new('RGBA', (size, size), (139,69,19,255))  # Default brown
        atlas.paste(img, (f * size, 0))
    atlas_path = f'../assets/graphics/tiles/{tile_id}_atlas.png'
    atlas.save(atlas_path)
    return [f"{tile_id}_frame{i}.png" for i in range(frames)]

def generate_data():
    # Items gen
    items_dir = 'items/'
    all_items = []
    for csv_file in glob.glob(items_dir + '*.csv'):
        cat = os.path.basename(csv_file).split('.')[0]
        df = pd.read_csv(csv_file)
        df['category'] = cat

        if cat == 'books':
            spells_list = []
            for _, row in df.iterrows():
                spells = SAMPLE_SPELLS.get(row['id'], [])
                spells_list.append(spells)
            df['contained_spells'] = spells_list
        elif cat == 'scrolls':
            spells_list = []
            for _, row in df.iterrows():
                spells = SAMPLE_SCROLL_SPELLS.get(row['id'], [])
                spells_list.append(spells)
            df['contained_spells'] = spells_list
        elif cat == 'spells':
            charges_list = []
            for _, row in df.iterrows():
                charges = SAMPLE_SPELL_CHARGES.get(row['id'], 1)
                charges_list.append(charges)
            df['charges'] = charges_list

        all_items.append(df)

    items_df = pd.concat(all_items, ignore_index=True)

    variants = []
    for idx, row in items_df.iterrows():
        if np.random.random() > 0.5 and row['durability'] != 'N/A':
            variant = row.copy()
            variant['durability'] = int(row['durability']) * 0.7
            variant['price'] = int(row['price']) * 0.8
            variant['id'] += '_rusted'
            variants.append(variant)

    items_df = pd.concat([items_df, pd.DataFrame(variants)], ignore_index=True)

    data = {'items': items_df.to_dict('records')}
    with open('../assets/data/items.json', 'w') as f:
        json.dump(data, f, indent=2)

    # Tiles gen
    tiles_df = pd.read_csv('data_sources/tiles.csv')
    tiles = []
    for _, row in tiles_df.iterrows():
        tile = {
            'id': row['id'],
            'type': row['type'],
            'preferred_layer': int(row['preferred_layer']),
            'passable': row['passable'] == 'true',
            'blocks_sight': row['passable'] != 'true',
            'description': row['description'],
            'flammability': 0
        }

        if row['id'] == 'water_wave':
            frames = generate_tile_atlas('water_wave', 4)
            tile['animation_frames'] = frames
            tile['animation_speed'] = 0.2
        elif row['id'] == 'grass_wispy':
            frames = generate_tile_atlas('grass_wispy', 2)
            tile['animation_frames'] = frames
            tile['animation_speed'] = 0.1
            tile['wind_sway'] = True
        elif row['id'] == 'grass_tuft':
            tile['flammability'] = 80
        elif row['id'] == 'tree_oak':
            tile['flammability'] = 90

        # Procedural heights (previous logic)
        if row['type'] == 'terrain' and np.random.random() < 0.2:
            tile['height_levels'] = [
                {'height': 0, 'passable': False, 'transparent': False, 'views': {'default': f"{row['id']}_trunk.png"}},
                {'height': 1, 'passable': True, 'transparent': True, 'views': {'default': f"{row['id']}_branches.png"}},
                {'height': 2, 'passable': True, 'transparent': True, 'views': {'default': f"{row['id']}_leaves.png"}}
            ]
        elif row['type'] == 'furniture':
            tile['supports_furniture'] = True
            tile['height_levels'] = [
                {'height': 0, 'passable': row['passable'] == 'true', 'transparent': row['transparent'] == 'true', 'views': {'default': f"{row['id']}_base.png"}},
                {'height': 1, 'passable': False, 'transparent': False, 'views': {'default': f"{row['id']}_top.png"}}
            ]
        elif row['type'] == 'bridge':
            tile['connects_layers'] = [row['preferred_layer'], row['preferred_layer'] + 1]
            tile['height_levels'] = [
                {'height': 0, 'passable': True, 'transparent': True, 'views': {'default': f"{row['id']}.png"}}
            ]
        else:
            tile['height_levels'] = [
                {'height': 0, 'passable': row['passable'] == 'true', 'transparent': row['transparent'] == 'true', 'views': {'default': f"{row['id']}.png"}}
            ]

        tiles.append(tile)

    with open('../assets/data/tilesets.json', 'w') as f:
        json.dump({
            'version': '1.4',
            'map_layers': 6,
            'tile_width': 64,
            'tile_height': 32,
            'tiles': tiles
        }, f, indent=2)

    # Biome map gen
    def generate_multi_layer_map(width=50, height=50, layers=6, seed=42):
        np.random.seed(seed)
        elev_noise = simple_perlin(width, height, seed)
        moist_noise = simple_perlin(width, height, seed + 1)
        map_data = {}
        for layer in range(layers):
            height_maps = []
            for x in range(width):
                row_heights = []
                for y in range(height):
                    if layer == 0:  # Caves
                        tile = "rock_boulder" if np.random.random() < 0.1 else "empty"
                    elif layer in [1, 2]:  # Ground biomes
                        biome = get_biome(elev_noise[x, y], moist_noise[x, y])
                        tile = np.random.choice(get_biome_tiles(biome))
                        if np.random.random() < 0.1:
                            tile = np.random.choice(["table_wooden", "chair_wooden"])
                    elif layer == 3:  # Bridges
                        tile = "bridge_rope" if x == 25 and 10 <= y < 40 else "empty"
                    else:
                        tile = "empty" if np.random.random() < 0.9 else "rock_boulder"

                    h_list = [{'height': 0, 'tile': tile}]
                    if tile == "tree_oak" and np.random.random() < 0.5:
                        h_list.append({'height': 1, 'tile': tile})
                    elif tile == "table_wooden":
                        h_list.append({'height': 1, 'tile': tile})
                    row_heights.append(h_list)
                height_maps.append(row_heights)
            map_data[layer] = height_maps
        return {'map': map_data, 'biomes': {'elev': elev_noise.tolist(), 'moist': moist_noise.tolist()}}

    map_json = generate_multi_layer_map()
    with open('../assets/data/map.json', 'w') as f:
        json.dump(map_json, f, indent=2)

if __name__ == '__main__':
    generate_data()
