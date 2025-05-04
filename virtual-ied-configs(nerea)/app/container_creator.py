import json
import subprocess
import glob
import os

def load_json_config(json_file):
    with open(json_file, 'r', encoding='utf-8') as f:
        return json.load(f)

def build_and_run_breaker_container(instance_name, interface):
    # Construye la imagen del contenedor para el breaker
    build_command = [
        "docker", "build", "../../virtual-circuit-breaker",
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
    try:
        data = load_json_config(json_file)
    except Exception as e:
        print(f"Error al cargar el JSON: {e}")
        return
    print("ESTOY AQUI", flush=True)
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

# if __name__ == "__main__":
#     json_file = "IOP_2019_HV_2.scd.json"
create_breaker_containers_from_json("IOP_2019_HV_2.scd.json")


# import json
# import subprocess

# def load_json_config(json_file):
#     with open(json_file, 'r', encoding='utf-8') as f:
#         return json.load(f)

# def build_and_run_container(node_name, interface):

#     build_command = [
#         "docker", "build", ".", 
#         "--build-arg", f"NODE_NAME={node_name}",
#         "--build-arg", f"INTERFACE={interface}",
#         "-t", f"virtual-ied-{node_name}"
#     ]
#     subprocess.run(build_command, check=True)

#     run_command = [
#         "docker", "run", "-d", 
#         "--name", f"virtual-ied-{node_name}",
#         f"virtual-ied-{node_name}"
#     ]
#     subprocess.run(run_command, check=True)

# def create_containers_from_json(json_file):
#     data = load_json_config(json_file)
    
#     print("HOLITA ESTOY AQUI SOSOSOSOOS \n")
#     for ied in data:
#         for ap in ied.get("AccessPoints", []):
#             for ld in ap.get("LogicalDevices", []):
#                 for ln in ld.get("LogicalNodes", []):
#                     node_name = ln["lnType"]
#                     print(f"Levantando contenedor para el nodo lógico: {node_name}")

#                     interface = "eth0"
#                     build_and_run_container(node_name, interface)

# if __name__ == "__main__":
#     json_file = "ied-config.json" 
#     create_containers_from_json(json_file)
