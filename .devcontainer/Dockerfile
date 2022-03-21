ARG VARIANT="focal"

FROM mcr.microsoft.com/vscode/devcontainers/base:0-${VARIANT}

RUN apt-get update

RUN apt-get install --no-install-recommends --yes man-db tcpdump net-tools dnsutils telnet traceroute iputils-ping

RUN apt-get install --no-install-recommends --yes curl make

RUN apt-get update && apt-get install --yes gcc
