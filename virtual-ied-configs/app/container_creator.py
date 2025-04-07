import json
import subprocess

def load_json_config(json_file):
    with open(json_file, 'r', encoding='utf-8') as f:
        return json.load(f)

def build_and_run_container(node_name, interface):

    build_command = [
        "docker", "build", ".", 
        "--build-arg", f"NODE_NAME={node_name}",
        "--build-arg", f"INTERFACE={interface}",
        "-t", f"virtual-ied-{node_name}"
    ]
    subprocess.run(build_command, check=True)

    run_command = [
        "docker", "run", "-d", 
        "--name", f"virtual-ied-{node_name}",
        f"virtual-ied-{node_name}"
    ]
    subprocess.run(run_command, check=True)

def create_containers_from_json(json_file):
    data = load_json_config(json_file)
    
    for ied in data:
        for ap in ied.get("AccessPoints", []):
            for ld in ap.get("LogicalDevices", []):
                for ln in ld.get("LogicalNodes", []):
                    node_name = ln["lnType"]
                    print(f"Levantando contenedor para el nodo lógico: {node_name}")

                    interface = "eth0"
                    build_and_run_container(node_name, interface)

if __name__ == "__main__":
    json_file = "ied-config.json" 
    create_containers_from_json(json_file)
