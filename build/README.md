# Build instructions

```
export HALON_REPO_USER=exampleuser
export HALON_REPO_PASS=examplepass
docker-compose -p halon-extras-file-pickup up --build
docker-compose -p halon-extras-file-pickup down --rmi local
```