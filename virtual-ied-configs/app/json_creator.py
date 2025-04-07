import xml.etree.ElementTree as ET
import json
import glob
import os

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
        
        print(f'Archivo JSON generado correctamente: ied-config.json')
    
    except ET.ParseError as e:
        print(f'Error al parsear el archivo XML: {e}')
    except FileNotFoundError as e:
        print(f'Archivo no encontrado: {e}')
    except Exception as e:
        print(f'Ocurrió un error: {e}')

def process_all_scd_files(directory, output_json):
    # Buscar todos los archivos que terminan con '.scd.xml' en el directorio especificado
    scd_files = glob.glob(os.path.join(directory, "*.xml"))
    print("QUE OSTIAS NEREA \n")   
    if not scd_files:
        print("No se encontraron archivos .xml en el directorio.")
        return
    
    for xml_file in scd_files:
        print(f"Procesando archivo: {xml_file}")
        scl_to_json(xml_file, output_json)
    print("SE CREO EL JSON A TRASTEAR :D \n")

process_all_scd_files('.', 'ied-config.json')

# import xml.etree.ElementTree as ET
# import json

# def scl_to_json(xml_file, output_file):
# 		try:
# 			tree = ET.parse(xml_file)
# 			root = tree.getroot()
# 			namespaces = {'scl': 'http://www.iec.ch/61850/2003/SCL'}
			
# 			data = []

# 			ieds = root.findall('scl:IED', namespaces)
# 			with open(output_file, 'w') as f:
# 				for ied in ieds:
# 					# ied_name = ied.get('name')
# 					# f.write(f'IED: {ied_name}\n')
# 					access_points = ied.findall('scl:AccessPoint', namespaces)
# 					for ap in access_points:
# 						servers = ap.findall('scl:Server', namespaces)
# 						for server in servers:
# 							logical_devices = server.findall('scl:LDevice', namespaces)
# 							for ld in logical_devices:
# 								ld_name = ld.get('inst')
# 								f.write(f'Logical Device: {ld_name}\n')
# 								zero_node = ld.find('scl:LN0', namespaces)
# 								zero_name = zero_node.get('lnClass')
# 								zero_type = zero_node.get('lnType')
# 								zero_inst = zero_node.get('inst')
# 								zero_desc = zero_node.get('desc')
# 								f.write(f'   {zero_name} \n      type: {zero_type} inst: {zero_inst} desc: {zero_desc}\n')
# 								#EN BUSCA DEL TIPO DE DATO
# 								data_type_templates = root.findall('scl:DataTypeTemplates', namespaces)
# 								for data_type_template in data_type_templates:
# 									LNode_type_templates = data_type_template.findall('scl:LNodeType', namespaces)
# 									for LNode_type_template in LNode_type_templates:
# 										if (LNode_type_template.get('id') == zero_node.get('lnType')):
# 											data_types = LNode_type_template.findall('scl:DO', namespaces)
# 											for data_type in data_types:
# 												do_name = data_type.get('name')
# 												do_type = data_type.get('type')
# 												f.write(f'            DO: {do_name} type: {do_type} \n')
# 												do_type_direct = root.find(f'scl:DataTypeTemplates/scl:DOType[@id="{do_type}"]', namespaces)
# 												#do_type_direct = data_type.find('scl:DOType id="', {do_type}, namespaces)
# 												if do_type_direct is not None:
# 													cdc = do_type_direct.get('cdc')
# 													f.write(f'               DOType: {do_type} cdc: {cdc}\n')
# 													das = do_type_direct.findall('scl:DA', namespaces)
# 													for da in das:
# 														da_name = da.get('name')
# 														da_type = da.get('type')
# 														da_bType = da.get('bType')
# 														da_fc = da.get('fc')
# 														f.write(f'               DA: {da_name} type: {da_type} btype: {da_bType} fc: {da_fc} \n')
# 													if da_type is not None:
# 														da_type_direct = root.find(f'scl:DataTypeTemplates/scl:DAType[@id="{da_type}"]', namespaces)
# 														if da_type_direct is not None:
# 															f.write(f'                         DAType: {da_type}')
# 															bdas = da_type_direct.findall('scl:BDA', namespaces)
# 															for bda in bdas:
# 																bda_name = bda.get('name')
# 																bda_type = bda.get('type')
# 																bda_bType = bda.get('bType')
# 																bda_valKind = bda.get('valKind')
# 																f.write(f'                        		BDA: {bda_name} type: {bda_type} btype: {bda_bType} valKind: {bda_valKind} \n')
# 								data_sets = zero_node.findall('scl:DataSet', namespaces)
# 								for data_set in data_sets:
# 									ds_name = data_set.get('name')
# 									f.write(f'      Data Set: {ds_name}\n')
# 									fcs = data_set.findall('scl:FCDA', namespaces)
# 									for fc in fcs:
# 										fc_name = fc.get('ldInst')
# 										fc_class = fc.get('lnClass')
# 										fc_type = fc.get('fc')
# 										fc_prefix = fc.get('prefix')
# 										fc_ln_inst = fc.get('lnInst')
# 										fc_do_name = fc.get('doName')
# 										f.write(f'        FC: {fc_name} class: {fc_class} type: {fc_type} prefix: {fc_prefix} lnInst: {fc_ln_inst} doName: {fc_do_name}\n')
# 								logical_nodes = ld.findall('scl:LN', namespaces)
# 								for ln in logical_nodes:
# 									ln_class = ln.get('lnClass')
# 									ln_type = ln.get('lnType')
# 									ln_inst = ln.get('inst')
# 									f.write(f'    Logical Node:\n\t\t class: {ln_class} type: {ln_type} inst: {ln_inst}\n')
# 									data_set= ln.findall('scl:DataSet', namespaces)
# 									for ds in data_set:
# 										print('HOLA')
# 										ds_name = ds.get('name')
# 										f.write(f'    Data Set: {ds_name}\n')
# 										fc = ds.findall('scl:FC', namespaces)
# 										for f in fc:
# 											fc_name = f.get('name')
# 											f.write(f'      FC: {fc_name}\n')
			
# 			print(f'Información de IEDs y nodos lógicos escrita en {output_file}')
# 		except ET.ParseError as e:
# 			print(f'Error al parsear el archivo XML: {e}')
# 		except FileNotFoundError as e:
# 			print(f'Archivo no encontrado: {e}')
# 		except Exception as e:
# 			print(f'Ocurrió un error: {e}')

# # Uso de la función
# find_ieds_and_logical_nodes('IOP_2019_HV_2.scd.xml', 'logical_nodes.txt')


#  rptena: report habilitado o no? Liberia mms
# #  rptena: report habilitado o no? Liberia mms
