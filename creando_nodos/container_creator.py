import json
import subprocess
import os
import sys

DEVNULL = subprocess.DEVNULL


def load_json_config(json_file):
    with open(json_file, 'r', encoding='utf-8') as f:
        return json.load(f)


def ensure_network(network_name):
    """
    Ensure that a Docker network exists; create it if absent.
    """
    try:
        subprocess.run([
            "docker", "network", "inspect", network_name
        ], check=True, stdout=DEVNULL, stderr=DEVNULL)
        print(f"La red Docker '{network_name}' ya existe.", flush=True)
    except subprocess.CalledProcessError:
        print(f"Creando la red Docker '{network_name}'...", flush=True)
        subprocess.run([
            "docker", "network", "create", network_name
        ], check=True)
        print(f"Red Docker '{network_name}' creada.", flush=True)


def build_and_run_container(prefix, context_subdir, instance_name, interface, network_mode=None):
    base_path = os.path.dirname(os.path.abspath(__file__))
    context_path = os.path.join(base_path, "..", context_subdir)
    container_name = f"{prefix}-{instance_name}"

    # Eliminar contenedor previo si existe
    subprocess.run([
        "docker", "rm", "-f", container_name
    ], check=False, stdout=DEVNULL, stderr=DEVNULL)
    print(f"Contenedor {container_name} eliminado previamente (si existía).", flush=True)

    # Construir la imagen
    image_tag = f"{prefix}:{instance_name}"
    build_cmd = [
        "docker", "build", context_path,
        "--build-arg", f"NODE_NAME={instance_name}",
        "--build-arg", f"INTERFACE={interface}",
        "-t", image_tag
    ]
    print("Ejecutando comando de build:", " ".join(build_cmd), flush=True)
    subprocess.run(build_cmd, check=True)

    # Unirse a la red si es necesario
    if network_mode and network_mode.lower() != "host":
        ensure_network(network_mode)

    # Ejecutar el contenedor
    run_cmd = [
        "docker", "run", "-d", "--name", container_name
    ]
    if network_mode:
        run_cmd += ["--network", network_mode]
    run_cmd += [image_tag]
    print("Ejecutando comando de run:", " ".join(run_cmd), flush=True)
    subprocess.run(run_cmd, check=True)


def create_containers_from_json(json_file):
    try:
        data = load_json_config(json_file)
    except Exception as e:
        print(f"Error al cargar el JSON: {e}")
        return

    # Recopilar nodos breakers (XCBR) y puntos de acceso para readers
    breakers = []
    readers = []
    for ied in data:
        # Collect LogicalNodes of class XCBR
        for ap in ied.get("AccessPoints", []):
            for ld in ap.get("LogicalDevices", []):
                for ln in ld.get("LogicalNodes", []):
                    if ln.get("lnClass") == "XCBR":
                        breakers.append(ln)
        # Collect each AccessPoint as a reader
        readers.extend(ied.get("AccessPoints", []))

    # Lanzar contenedores de readers
    print(f"📡 Se encontraron {len(readers)} readers (puntos de acceso) en el JSON.", flush=True)
    for idx, ap in enumerate(readers, start=1):
        instance = f"reader-{idx}"
        print(f"Levantando contenedor para {instance} (AccessPoint={ap.get('name','')})...", flush=True)
        build_and_run_container(
            prefix="virtual-104-gtw",
            context_subdir="virtual-104-gtw",
            instance_name=instance,
            interface="eth0",
            network_mode="sv_network"
        )

    # Lanzar contenedores de breakers
    print(f"🎚️ Se encontraron {len(breakers)} breakers (XCBR) en el JSON.", flush=True)
    for idx, ln in enumerate(breakers, start=1):
        instance = f"breaker-{idx}"
        print(f"Levantando contenedor para {instance}...", flush=True)
        build_and_run_container(
            prefix="virtual-circuit-breaker",
            context_subdir="virtual-circuit-breaker",
            instance_name=instance,
            interface="eth0",
            network_mode="sv_network"
        )


if __name__ == "__main__":
    base_path = os.path.dirname(os.path.abspath(__file__))
    json_file = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        base_path, "archivos_xml", "IOP_2019_HV_2.scd.json"
    )

    print("Usando JSON en:", json_file, flush=True)
    ensure_network("sv_network")
    create_containers_from_json(json_file)

    # Eliminar JSON tras ejecución
    try:
        os.remove(json_file)
        print("Archivo JSON eliminado.", flush=True)
    except Exception as e:
        print(f"Error al eliminar el archivo JSON: {e}", flush=True)
