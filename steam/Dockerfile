# A Dockerfile that prepares the Steam Sniper environment to build Endless Sky from source.

# Use the latest Sniper environment from Steam.
FROM registry.gitlab.steamos.cloud/steamrt/sniper/sdk:latest

ARG arch

# The only dependency missing from the runtime is libmad, so install it.
RUN apt-get install -yq --no-install-recommends libmad0-dev:${arch}

WORKDIR /endless-sky
CMD ["/bin/bash"]
