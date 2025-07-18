# #!/usr/bin/env python3
# import sys, os, glob, json, yaml, copy

# CLASSES       = ['XCBR', 'MMXU', 'USVC']
# SERVICE_MAP   = {'XCBR': 'virtual-circuit-breaker', 'MMXU': 'virtual-ied', 'USVC': 'virtual-104-gtw'}
# PREFIX_MAP    = {'XCBR': 'XBCR', 'MMXU': 'reader', 'USVC': 'usvc'}
# CLASS_DEPENDS = {'XCBR': [], 'MMXU': [], 'USVC': []}

# # === Entrada ===
# if len(sys.argv) > 1 and os.path.isfile(sys.argv[1]):
#     json_file = sys.argv[1]
# else:
#     matches = glob.glob(os.path.join('creando_nodos','archivos_xml','*.json'))
#     if not matches:
#         print("Error: no se encontró JSON en creando_nodos/archivos_xml/*.json")
#         sys.exit(1)
#     json_file = matches[0]

# print("➜ JSON de nodos:", json_file)

# base_compose = os.path.join(os.getcwd(), 'docker-compose-base.yml')
# if not os.path.isfile(base_compose):
#     print(f"Error: no existe {base_compose}")
#     sys.exit(1)
# print("➜ Compose base:", base_compose)

# with open(base_compose, 'r', encoding='utf-8') as f:
#     base = yaml.safe_load(f)

# base_services = base.get('services', {})

# # === Cargar plantillas base ===
# templates = {}
# for cls, svc in SERVICE_MAP.items():
#     if svc not in base_services:
#         print(f"Error: servicio base '{svc}' no está en docker-compose.yml")
#         sys.exit(1)
#     templates[cls] = base_services.pop(svc)

# # === Cargar datos JSON ===
# with open(json_file, 'r', encoding='utf-8') as f:
#     full_data = json.load(f)

# ied_list = full_data.get("IEDs", [])
# networks = full_data.get("Networks", {})

# ied_to_networks = {}
# for net_name, ied_names in networks.items():
#     for ied_name in ied_names:
#         try:
#             ied_num = int(''.join(filter(str.isdigit, ied_name)))
#         except ValueError:
#             continue
#         if ied_num not in ied_to_networks:
#             ied_to_networks[ied_num] = []
#         ied_to_networks[ied_num].append(net_name)

# # === Extraer elementos ===
# elements_by_ied = {}

# for idx, ied in enumerate(ied_list, start=1):
#     for ap in ied.get('AccessPoints', []):
#         for ld in ap.get('LogicalDevices', []):
#             ln0 = ld.get('LN0')
#             if isinstance(ln0, dict) and ln0.get('lnClass') in CLASSES:
#                 ln0['_IED'] = idx
#                 elements_by_ied.setdefault(idx, []).append(ln0)
#             for ln in ld.get('LogicalNodes', []):
#                 if ln.get('lnClass') in CLASSES:
#                     ln['_IED'] = idx
#                     elements_by_ied.setdefault(idx, []).append(ln)

# # === Asignar servicios sin IED al primero ===
# unassigned = []
# for ied_elems in elements_by_ied.values():
#     for e in ied_elems:
#         if '_IED' not in e:
#             unassigned.append(e)
# if unassigned:
#     print("➜ Asignando servicios no etiquetados al IED 1")
#     elements_by_ied.setdefault(1, []).extend(unassigned)

# # === Eliminar docker-compose anteriores ===
# for f in glob.glob("docker-compose-*.yml"):
#     if os.path.basename(f) == "docker-compose-base.yml":
#         continue
#     try:
#         os.remove(f)
#         print(f"🗑️  Eliminado antiguo: {f}")
#     except Exception as e:
#         print(f"⚠️  No se pudo eliminar {f}: {e}")

# # === Crear compose por IED ===
# output_files = []
# global_class_counters = {cls: 0 for cls in CLASSES}
# for ied_num, elems in elements_by_ied.items():
#     dynamic_services = {}

#     counts = {cls: 0 for cls in CLASSES}
#     for e in elems:
#         cls = e.get('lnClass')
#         if cls in counts:
#             counts[cls] += 1

#     # Crear servicios dinámicos por clase
#     for cls, total in counts.items():
#         template = templates[cls]
#         for _ in range(total):
#             global_class_counters[cls] += 1
#             global_index = global_class_counters[cls]

#             base_name = SERVICE_MAP[cls]
#             svc_key = base_name if total == 1 else f"{base_name}-{global_index}"
#             cfg = copy.deepcopy(template)
#             cfg['container_name'] = f"{PREFIX_MAP[cls]}-{global_index}"

#             # Imagen única por IED y clase
#             base_img = template.get('image', 'substation_virtual_nodes')
#             if isinstance(base_img, list):
#                 base_img = base_img[0]
#             cfg['image'] = f"ied{ied_num}-{base_img}-{svc_key}"

#             if ied_num in ied_to_networks:
#                 cfg['networks'] = list(set(ied_to_networks[ied_num]))

#             deps = []
#             for dep in CLASS_DEPENDS.get(cls, []):
#                 for j in range(1, counts[dep] + 1):
#                     dep_name = SERVICE_MAP[dep] if counts[dep] == 1 else f"{SERVICE_MAP[dep]}-{j}"
#                     deps.append(dep_name)
#             if deps:
#                 cfg['depends_on'] = deps

#             dynamic_services[svc_key] = cfg

#     compose_networks = {}
#     if ied_num in ied_to_networks:
#         for net in ied_to_networks[ied_num]:
#             compose_networks[net] = {'driver': 'bridge'}

#     # Merge with any static services (e.g., merging unit)
#     special_services = {}
#     # for special in ['virtual-merging-unit', 'smvgentime']:
#     #     if special in base_services:
#     #         static_cfg = copy.deepcopy(base_services[special])
#     #         static_cfg['networks'] = list(compose_networks.keys())
#     #         special_services[special] = static_cfg
    
#     if ied_num == 1:
#         for special in ['virtual-merging-unit', 'smvgentime']:
#             if special in base_services:
#                 static_cfg = copy.deepcopy(base_services[special])
#                 static_cfg['networks'] = list(compose_networks.keys())
#                 # Añadir o modificar 'image' para prefijar con ied1-
#                 img_name = static_cfg.get('image') or special  # fallback a nombre del servicio
#                 static_cfg['image'] = f"ied1-{img_name}"
#                 special_services[special] = static_cfg

#     new_compose = {
#         'services': {**special_services, **dynamic_services},
#         'networks': compose_networks
#     }

#     output_path = os.path.join(os.getcwd(), f'docker-compose-{ied_num}.yml')
#     with open(output_path, 'w', encoding='utf-8') as f:
#         yaml.dump(new_compose, f, sort_keys=False, default_flow_style=False)

#     output_files.append(output_path)

# print(f"\n✅ Composes generados para {len(output_files)} IEDs:")
# for f in output_files:
#     print(f"   • {os.path.basename(f)}")

#AHORA METEMOS EL ARTIFACTORY
#!/usr/bin/env python3
import sys, os, glob, json, yaml, copy
import requests
from getpass import getpass
from requests.auth import HTTPBasicAuth
import subprocess
import shutil

mode = None
json_file = None
artifactory_repo = None

for arg in sys.argv[1:]:
    if arg in ['local', 'artifactory']:
        mode = arg
    elif os.path.isfile(arg) and arg.endswith('.json'):
        json_file = arg

# Si no se pasó modo, preguntar
if mode not in ['local', 'artifactory']:
    while mode not in ['local', 'artifactory']:
        mode = input("¿Qué modo deseas usar? [local/artifactory]: ").strip().lower()

if mode == 'artifactory':
    artifactory_repo = input("📦 Nombre del repositorio Docker (ej: virtualizaciondenodoslogicos-docker-dev-local.artifact.tecnalia.dev:443): ").strip()
    artifactory_tag = input("🏷️ Tag de las imágenes (default: latest): ").strip() or "latest"


# Si no se pasó JSON, buscarlo automáticamente
if not json_file:
    matches = glob.glob(os.path.join('creando_nodos','archivos_xml','*.json'))
    if not matches:
        print("Error: no se encontró JSON en creando_nodos/archivos_xml/*.json")
        sys.exit(1)
    json_file = matches[0]

print("➜ Modo:", mode)


CLASSES       = ['XCBR', 'MMXU', 'USVC']
SERVICE_MAP   = {'XCBR': 'virtual-circuit-breaker', 'MMXU': 'virtual-ied', 'USVC': 'virtual-104-gtw'}
PREFIX_MAP    = {'XCBR': 'XBCR', 'MMXU': 'reader', 'USVC': 'usvc'}
CLASS_DEPENDS = {'XCBR': [], 'MMXU': [], 'USVC': []}

# === Entrada ===

# Si estamos en modo Artifactory, preguntar qué imagen usar para cada clase

if len(sys.argv) > 1 and os.path.isfile(sys.argv[1]):
    json_file = sys.argv[1]
else:
    matches = glob.glob(os.path.join('creando_nodos','archivos_xml','*.json'))
    if not matches:
        print("Error: no se encontró JSON en creando_nodos/archivos_xml/*.json")
        sys.exit(1)
    json_file = matches[0]

print("➜ JSON de nodos:", json_file)

base_compose = os.path.join(os.getcwd(), 'docker-compose-base.yml')
if not os.path.isfile(base_compose):
    print(f"Error: no existe {base_compose}")
    sys.exit(1)
print("➜ Compose base:", base_compose)

with open(base_compose, 'r', encoding='utf-8') as f:
    base = yaml.safe_load(f)

base_services = base.get('services', {})

# === Cargar plantillas base ===
templates = {}
for cls, svc in SERVICE_MAP.items():
    if svc not in base_services:
        print(f"Error: servicio base '{svc}' no está en docker-compose.yml")
        sys.exit(1)
    templates[cls] = base_services.pop(svc)

# === Cargar datos JSON ===
with open(json_file, 'r', encoding='utf-8') as f:
    full_data = json.load(f)

ied_list = full_data.get("IEDs", [])
networks = full_data.get("Networks", {})

ied_to_networks = {}
for net_name, ied_names in networks.items():
    for ied_name in ied_names:
        try:
            ied_num = int(''.join(filter(str.isdigit, ied_name)))
        except ValueError:
            continue
        if ied_num not in ied_to_networks:
            ied_to_networks[ied_num] = []
        ied_to_networks[ied_num].append(net_name)

# === Extraer elementos ===
elements_by_ied = {}

for idx, ied in enumerate(ied_list, start=1):
    for ap in ied.get('AccessPoints', []):
        for ld in ap.get('LogicalDevices', []):
            ln0 = ld.get('LN0')
            if isinstance(ln0, dict) and ln0.get('lnClass') in CLASSES:
                ln0['_IED'] = idx
                elements_by_ied.setdefault(idx, []).append(ln0)
            for ln in ld.get('LogicalNodes', []):
                if ln.get('lnClass') in CLASSES:
                    ln['_IED'] = idx
                    elements_by_ied.setdefault(idx, []).append(ln)

# === Asignar servicios sin IED al primero ===
unassigned = []
for ied_elems in elements_by_ied.values():
    for e in ied_elems:
        if '_IED' not in e:
            unassigned.append(e)
if unassigned:
    print("➜ Asignando servicios no etiquetados al IED 1")
    elements_by_ied.setdefault(1, []).extend(unassigned)

# === Eliminar docker-compose anteriores ===
for f in glob.glob("docker-compose-*.yml"):
    if os.path.basename(f) == "docker-compose-base.yml":
        continue
    try:
        os.remove(f)
        print(f"🗑️  Eliminado antiguo: {f}")
    except Exception as e:
        print(f"⚠️  No se pudo eliminar {f}: {e}")


# === Crear compose por IED ===
output_files = []
global_class_counters = {cls: 0 for cls in CLASSES}
for ied_num, elems in elements_by_ied.items():
    dynamic_services = {}

    counts = {cls: 0 for cls in CLASSES}
    for e in elems:
        cls = e.get('lnClass')
        if cls in counts:
            counts[cls] += 1

    # Crear servicios dinámicos por clase
    for cls, total in counts.items():
        template = templates[cls]
        for _ in range(total):
            global_class_counters[cls] += 1
            global_index = global_class_counters[cls]

            base_name = SERVICE_MAP[cls]
            svc_key = base_name if total == 1 else f"{base_name}-{global_index}"
            cfg = copy.deepcopy(template)
            cfg['container_name'] = f"{PREFIX_MAP[cls]}-{global_index}"

            # Imagen única por IED y clase
            base_img = template.get('image', 'substation_virtual_nodes')
            if isinstance(base_img, list):
                base_img = base_img[0]

            if mode == 'local':
                cfg['image'] = f"ied{ied_num}-{base_img}-{svc_key}"
            else:
                image_name = f"ied{ied_num}-{base_img}-{svc_key}"
                cfg['image'] = f"{artifactory_repo}/{image_name}:{artifactory_tag}"

            if ied_num in ied_to_networks:
                cfg['networks'] = list(set(ied_to_networks[ied_num]))

            deps = []
            for dep in CLASS_DEPENDS.get(cls, []):
                for j in range(1, counts[dep] + 1):
                    dep_name = SERVICE_MAP[dep] if counts[dep] == 1 else f"{SERVICE_MAP[dep]}-{j}"
                    deps.append(dep_name)
            if deps:
                cfg['depends_on'] = deps

            dynamic_services[svc_key] = cfg

    compose_networks = {}
    if ied_num in ied_to_networks:
        for net in ied_to_networks[ied_num]:
            compose_networks[net] = {'driver': 'bridge'}

    # Merge with any static services (e.g., merging unit)
    special_services = {}
    # for special in ['virtual-merging-unit', 'smvgentime']:
    #     if special in base_services:
    #         static_cfg = copy.deepcopy(base_services[special])
    #         static_cfg['networks'] = list(compose_networks.keys())
    #         special_services[special] = static_cfg
    
    if ied_num == 1:
        for special in ['virtual-merging-unit', 'smvgentime']:
            if special in base_services:
                static_cfg = copy.deepcopy(base_services[special])
                static_cfg['networks'] = list(compose_networks.keys())
                # Añadir o modificar 'image' para prefijar con ied1-
                img_name = static_cfg.get('image') or special  # fallback a nombre del servicio
                if mode == 'local':
                    static_cfg['image'] = f"ied1-{img_name}"
                else:
                    image_name = img_name.replace("_", "-")
                    static_cfg['image'] = f"{artifactory_repo}/ied1-{image_name}:latest"
                special_services[special] = static_cfg

    new_compose = {
        'services': {**special_services, **dynamic_services},
        'networks': compose_networks
    }

    output_path = os.path.join(os.getcwd(), f'docker-compose-{ied_num}.yml')
    with open(output_path, 'w', encoding='utf-8') as f:
        yaml.dump(new_compose, f, sort_keys=False, default_flow_style=False)

    output_files.append(output_path)

print(f"\n✅ Composes generados para {len(output_files)} IEDs:")
for f in output_files:
    print(f"   • {os.path.basename(f)}")

print("\nterraform Ejecutando exportador de imágenes y compose...")
subprocess.run(["python3", "creando_nodos/exportar_images.py", mode], check=True)

# terraform_dir = "terraform_artifactory" if mode == "artifactory" else "terraform"

# print(f" Ejecutando Terraform en {terraform_dir}...")
# subprocess.run(["terraform", "init"], cwd=terraform_dir, check=True)
# subprocess.run(["terraform", "apply", "-auto-approve"], cwd=terraform_dir, check=True)

terraform_dir = "terraform_artifactory" if mode == "artifactory" else "terraform"
print(f" Ejecutando Terraform en {terraform_dir}...")


# Limpieza segura si quieres asegurarte de un estado limpio
for item in [".terraform", "terraform.tfstate", "terraform.tfstate.backup"]:
    path = os.path.join(terraform_dir, item)
    if os.path.isdir(path):
        shutil.rmtree(path)
    elif os.path.isfile(path):
        os.remove(path)

try:
    subprocess.run(["terraform", "init"], cwd=terraform_dir, check=True)
    subprocess.run([
        "terraform", "apply",
        "-auto-approve",
        "-replace=openstack_networking_router_interface_v2.router_interface",
        "-replace=openstack_networking_floatingip_v2.fip",
        "-replace=openstack_networking_floatingip_associate_v2.fip_assoc"
    ], cwd=terraform_dir, check=True)
except subprocess.CalledProcessError as e:
    print(f"❌ Error ejecutando Terraform: {e}")
