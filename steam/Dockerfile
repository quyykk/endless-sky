# A Dockerfile that prepares the Steam Scout environment to build Endless Sky from source.

ARG arch

# Use the latest Scout environment from Steam.
FROM registry.gitlab.steamos.cloud/steamrt/scout/sdk${arch}:latest

# The only dependency missing from the runtime is libmad, so install it.
RUN apt-get install -yq --no-install-recommends libmad0-dev

WORKDIR /endless-sky
CMD ["/bin/bash"]
