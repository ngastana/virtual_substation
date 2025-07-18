#!/usr/bin/env python3
import yaml, glob, os, subprocess, sys

if len(sys.argv) < 2 or sys.argv[1] not in ['local', 'artifactory']:
    print("⚠️ Modo no especificado. Usa: local o artifactory")
    sys.exit(1)

mode = sys.argv[1]
print(f"\n Modo de exportación: {mode}")

DEST_DIR = "terraform_artifactory/" if mode == 'artifactory' else "terraform/"
os.makedirs(DEST_DIR, exist_ok=True)

for compose_path in glob.glob("docker-compose-*.yml"):
    if "base" in compose_path or "volumenes" in compose_path:
        continue

    ied_id = os.path.splitext(os.path.basename(compose_path))[0].split("-")[-1]
    ied_dir = os.path.join(DEST_DIR, f"ied{ied_id}")
    os.makedirs(ied_dir, exist_ok=True)

    print(f"\n Procesando {compose_path} → IED {ied_id}")

    with open(compose_path, "r", encoding="utf-8") as f:
        compose = yaml.safe_load(f)

    services = compose.get("services", {})
    updated_services = {}

    images = set()
    for name, service in compose.get("services", {}).items():
        img = service.get("image")

        if not img:
            if name in ["virtual-merging-unit", "smvgentime"]:
                img = f"ied{ied_id}-{name}"
            else:
                img = f"ied{ied_id}-substation_virtual_nodes-{name}"

        images.add(img)

        service.pop("build", None)
        service["image"] = img
        updated_services[name] = service

    compose["services"] = updated_services
    new_compose_path = os.path.join(ied_dir, "docker-compose.yml")
    with open(new_compose_path, "w", encoding="utf-8") as f:
        yaml.dump(compose, f, default_flow_style=False)
    print(f"✅ Compose modificado y copiado: {new_compose_path}")

    if mode == 'local':
        print(f"🔧 Construyendo imágenes...")
        subprocess.run(["docker", "compose", "-f", compose_path, "build"], check=True)

        for img in images:
            tar_path = os.path.join(ied_dir, f"{img}.tar")
            print(f"📦 Exportando imagen {img} → {tar_path}")
            subprocess.run(["docker", "save", "-o", tar_path, img], check=True)
    else:
        print(f" Modo Artifactory: solo se copian los compose. No se construyen imágenes.")
