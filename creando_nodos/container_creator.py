import json
import subprocess
import glob
import os

def load_json_config(json_file):
    with open(json_file, 'r', encoding='utf-8') as f:
        return json.load(f)

def build_and_run_breaker_container(instance_name, interface):
    # Construye la imagen del contenedor para el breaker
    base_path = os.path.dirname(os.path.abspath(__file__))
    context_path = os.path.join(base_path, "..", "virtual-circuit-breaker")
    print("Context path:", context_path, flush=True)

    container_name = f"virtual-circuit-breaker-{instance_name}"
    
    try:
        subprocess.run(["docker", "rm", "-f", container_name],
                       check=True,
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        print(f"Contenedor {container_name} eliminado previamente.", flush=True)
    except Exception as e:
        pass   
    
    build_command = [
        "docker", "build", context_path,
        "--build-arg", f"NODE_NAME={instance_name}",
        "--build-arg", f"INTERFACE={interface}",
        "-t", f"virtual-circuit-breaker:{instance_name}"
    ]
    print("Ejecutando comando de build:", " ".join(build_command))
    subprocess.run(build_command, check=True)

    # Levanta el contenedor con el nombre único
    run_command = [
        "docker", "run", "-d",
        "--name", f"virtual-circuit-breaker-{instance_name}",
        f"virtual-circuit-breaker:{instance_name}"
    ]
    print("Ejecutando comando de run:", " ".join(run_command))
    subprocess.run(run_command, check=True)

def create_breaker_containers_from_json(json_file):
    print("ESTOY AQUI", flush=True)
    try:
        data = load_json_config(json_file)
    except Exception as e:
        print(f"Error al cargar el JSON: {e}")
        return
    breaker_count = 0
    # Recorremos el JSON para contar los nodos lógicos de tipo "XCBR"
    for ied in data:
        for ap in ied.get("AccessPoints", []):
            for ld in ap.get("LogicalDevices", []):
                for ln in ld.get("LogicalNodes", []):
                    if ln.get("lnClass") == "XCBR":
                        breaker_count += 1

    print(f"Se encontraron {breaker_count} breakers (XCBR) en el JSON.", flush=True)

    for i in range(1, breaker_count + 1):
        instance_name = f"breaker-{i}"
        print(f"Levantando contenedor para {instance_name}...", flush=True)
        interface = "eth0"
        build_and_run_breaker_container(instance_name, interface)
  
if __name__ == "__main__":
    base_path = os.path.dirname(os.path.abspath(__file__))
    json_file = os.path.join(base_path, "archivos_xml", "IOP_2019_HV_2.scd.json")
    print("Usando JSON en:", json_file, flush=True)
    create_breaker_containers_from_json(json_file)
