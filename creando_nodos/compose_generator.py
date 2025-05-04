import sys
import os
import json
import yaml
import glob
import copy

glob_pattern = 'creando_nodos/archivos_xml/*.json'

# Clases a instanciar y mapeo a servicios base
CLASSES = ['XCBR', 'MMXU']
SERVICE_MAP = {
    'XCBR': 'virtual-circuit-breaker',
    'MMXU': 'virtual-104-gtw'
}

# Determinar ruta JSON
if len(sys.argv) > 1:
    json_file = sys.argv[1]
    if not os.path.isfile(json_file):
        print(f"Error: no existe el JSON especificado '{json_file}'")
        sys.exit(1)
else:
    matches = glob.glob(glob_pattern)
    if not matches:
        print(f"Error: no se encontró ningún JSON con patrón '{glob_pattern}'")
        sys.exit(1)
    json_file = matches[0]

print(f"Usando JSON de nodos: {json_file}")

# Determinar ruta al docker-compose base
def default_base_path():
    # Asume docker-compose.yml una carpeta arriba de este script
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'docker-compose-base.yml'))

if len(sys.argv) > 2:
    base_compose_file = sys.argv[2]
else:
    base_compose_file = default_base_path()

if not os.path.isfile(base_compose_file):
    print(f"Error: no se encontró el docker-compose base en '{base_compose_file}'")
    sys.exit(1)

print(f"Cargando docker-compose base desde: {base_compose_file}")

# Cargar docker-compose base
def load_base_compose(path):
    with open(path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)

base = load_base_compose(base_compose_file)
base_services = base.get('services', {})
base_networks = base.get('networks', {})

# Extraer plantillas para cada clase
templates = {}
for cls, svc_name in SERVICE_MAP.items():
    if svc_name not in base_services:
        print(f"Error: servicio base '{svc_name}' no existe en el compose base.")
        sys.exit(1)
    templates[cls] = base_services.pop(svc_name)

# Función para extraer nodos lógicos del JSON
def load_elements(json_path):
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    items = []
    for ied in data:
        for ap in ied.get('AccessPoints', []):
            for ld in ap.get('LogicalDevices', []):
                # LN0
                ln0 = ld.get('LN0')
                if isinstance(ln0, dict) and ln0.get('lnClass') in CLASSES:
                    items.append(ln0)
                # LogicalNodes
                for ln in ld.get('LogicalNodes', []):
                    if ln.get('lnClass') in CLASSES:
                        items.append(ln)
    return items

# Cargar y contar instancias
elements = load_elements(json_file)
counts = {cls: 0 for cls in CLASSES}
for e in elements:
    cls = e.get('lnClass')
    counts[cls] = counts.get(cls, 0) + 1

print("Instancias detectadas:")
for cls, num in counts.items():
    print(f"  - {cls}: {num}")

# Generar servicios dinámicos
dynamic_services = {}
for cls, num in counts.items():
    if num <= 0:
        continue
    template = templates[cls]
    for i in range(1, num + 1):
        name = f"{cls.lower()}_{i}"
        svc_conf = copy.deepcopy(template)
        svc_conf['container_name'] = name
        dynamic_services[name] = svc_conf

# Consolidar nuevo docker-compose
def build_new_compose(base, services, networks):
    return {
        'version': base.get('version', '3.8'),
        'services': {**services, **dynamic_services},
        'networks': networks
    }

new_compose = build_new_compose(base, base_services, base_networks)

# Guardar
output_file = '../docker-compose.yml'
with open(output_file, 'w', encoding='utf-8') as f:
    yaml.dump(new_compose, f, sort_keys=False, default_flow_style=False)

print(f"'{output_file}' creado con {len(new_compose['services'])} servicios.")
