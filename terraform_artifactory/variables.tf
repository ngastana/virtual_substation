variable "auth_url" { type = string }

variable "user_domain_name" { type = string }

variable "project_domain_id" { type = string }

variable "project_name" { type = string }

variable "user_name" { type = string }

variable "password" {
  type      = string
  sensitive = false
}

variable "region" { type = string }

variable "insecure" { type = bool }

variable "keypair_name" { type = string }

variable "public_key_path" { type = string }

variable "instance_name" { type = string }

variable "image_name" { type = string }

variable "flavor_name" { type = string }

variable "security_groups" { type = list(string) }

variable "network_name" { type = string }

variable "docker_volume_name" { type = string }

variable "docker_volume_size" { type = number }

variable "private_key_path" { type = string }

variable "images_tar_path" { type = string }

variable "docker_compose_file" { type = string }

#variable "cacert_file" { type = string }

variable "external_network_id" { type = string }

variable "artifactory_user" { type = string }

variable "artifactory_password" {
  type      = string
  sensitive = false
}
