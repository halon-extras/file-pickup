FROM --platform=linux/amd64 ubuntu:22.04
LABEL org.opencontainers.image.authors="support@halon.io"

ARG HALON_REPO_USER
ARG HALON_REPO_PASS

RUN apt-get update

RUN apt-get install -y wget gnupg
RUN apt-get install -y apt-transport-https

RUN wget -qO - https://raw.githubusercontent.com/halon/changelog/master/pgp-keys/7F0A73B5.asc | apt-key add -
RUN echo "deb https://repo.halon.io/ jammy stable" >> /etc/apt/sources.list.d/halon.list
RUN echo "machine repo.halon.io login ${HALON_REPO_USER} password ${HALON_REPO_PASS}" >> /etc/apt/auth.conf
RUN apt-get update && apt-get install -y halon=6.0.0 halon-extras-file-deliver=1.0.0

RUN /usr/bin/install -d /var/run/halon
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH

RUN apt-get install -y build-essential cmake git

RUN echo -n "UBU2204" > /OSRELEASE.txt

RUN mkdir /foo
RUN chmod 777 /foo
RUN mkdir /foo/foo
RUN chmod 777 /foo/foo
RUN mkdir /foo/bar
RUN chmod 777 /foo/bar
RUN mkdir /foo/baz
RUN chmod 777 /foo/baz

RUN apt-get install -y supervisor
COPY ./supervisord.conf /etc/supervisor/conf.d/supervisord.conf
CMD ["/usr/bin/supervisord"]