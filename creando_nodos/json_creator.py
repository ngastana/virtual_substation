import xml.etree.ElementTree as ET
import json
import glob
import os
import subprocess

def parse_data_type_templates(root, namespaces):
    #Extrae los DataTypeTemplates y los almacena en diccionarios
    data_type_templates = root.find('scl:DataTypeTemplates', namespaces)
    return {
        "lnode_types": {ln.get('id'): ln for ln in data_type_templates.findall('scl:LNodeType', namespaces)},
        "do_types": {do.get('id'): do for do in data_type_templates.findall('scl:DOType', namespaces)},
        "da_types": {da.get('id'): da for da in data_type_templates.findall('scl:DAType', namespaces)}
    }

def extract_ln_details(ln_type_id, lnode_types, do_types, namespaces):
    #Extrae los detalles de una clase LN
    details = []
    if ln_type_id in lnode_types:
        for do in lnode_types[ln_type_id].findall('scl:DO', namespaces):
            do_type_id = do.get('type')
            do_details = {"name": do.get('name', 'N/A'), "type": do_type_id, "Attributes": []}
            
            if do_type_id in do_types:
                for da in do_types[do_type_id].findall('scl:DA', namespaces):
                    do_details["Attributes"].append({
                        "name": da.get('name', 'N/A'),
                        "bType": da.get('bType', 'N/A'),
                        "fc": da.get('fc', 'N/A')
                    })
            
            details.append(do_details)
    return details

def scl_to_json(xml_file, output_json):
    try:
        tree = ET.parse(xml_file)
        root = tree.getroot()
        namespaces = {'scl': 'http://www.iec.ch/61850/2003/SCL'}
        print("SCL_TO_JSON \n")
        templates = parse_data_type_templates(root, namespaces)
        data = []
        
        for ied in root.findall('scl:IED', namespaces):
            ied_data = {"IED": ied.get('name', 'Desconocido'), "AccessPoints": []}
            
            for ap in ied.findall('scl:AccessPoint', namespaces):
                ap_data = {"LogicalDevices": []}
                
                for server in ap.findall('scl:Server', namespaces):
                    for ld in server.findall('scl:LDevice', namespaces):
                        ld_data = {"LogicalDevice": ld.get('inst', 'Desconocido'), "LN0": {}, "LogicalNodes": []}
                        
                        zero_node = ld.find('scl:LN0', namespaces)
                        if zero_node is not None:
                            ln_type_id = zero_node.get('lnType', 'N/A')
                            ld_data["LN0"] = {
                                "lnClass": zero_node.get('lnClass', 'N/A'),
                                "lnType": ln_type_id,
                                "inst": zero_node.get('inst', 'N/A'),
                                "desc": zero_node.get('desc', 'N/A'),
                                "Details": extract_ln_details(ln_type_id, templates["lnode_types"], templates["do_types"], namespaces)
                            }
                        
                        for ln in ld.findall('scl:LN', namespaces):
                            ln_type_id = ln.get('lnType', 'N/A')
                            ln_data = {
                                "lnClass": ln.get('lnClass', 'N/A'),
                                "lnType": ln_type_id,
                                "inst": ln.get('inst', 'N/A'),
                                "Details": extract_ln_details(ln_type_id, templates["lnode_types"], templates["do_types"], namespaces)
                            }
                            ld_data["LogicalNodes"].append(ln_data)
                        
                        ap_data["LogicalDevices"].append(ld_data)
                
                ied_data["AccessPoints"].append(ap_data)
            
            data.append(ied_data)
        
        with open(output_json, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        
        print(f'Archivo JSON generado correctamente: {output_json}')
    
    except ET.ParseError as e:
        print(f'Error al parsear el archivo XML: {e}')
    except FileNotFoundError as e:
        print(f'Archivo no encontrado: {e}')
    except Exception as e:
        print(f'Ocurrió un error: {e}')


def process_all_scd_files(directory):
    base_path = os.path.join(os.getcwd(), directory)
    xml_files = glob.glob(os.path.join(base_path, "*.xml"))
    if not xml_files:
        print("No se encontraron archivos XML en:", directory)
        exit(1)

    print("Archivos XML disponibles:")
    for idx, file in enumerate(xml_files, start=1):
        print(f"{idx}. {os.path.basename(file)}")

    # Se pide al usuario que elija el archivo a procesar
    try:
        choice = int(input("Seleccione el número del archivo a procesar: "))
        if choice < 1 or choice > len(xml_files):
            print("Número inválido. Saliendo.")
            exit(1)
    except ValueError:
        print("Entrada no válida. Saliendo.")
        exit(1)

    selected_xml = xml_files[choice - 1]
    base_name = os.path.splitext(os.path.basename(selected_xml))[0]
    output_json = os.path.join(directory, f"{base_name}.json")

    print(f"Procesando archivo: {os.path.basename(selected_xml)}")
    scl_to_json(selected_xml, output_json)
    print(f"Ejecutando container_creator.py con: {output_json}")
    subprocess.run(["python3", "./creando_nodos/container_creator.py", output_json], check=True)

if __name__ == "__main__":
    current_dir = os.getcwd()
    print("Directorio actual:", current_dir, flush=True)
    process_all_scd_files('creando_nodos/archivos_xml')
