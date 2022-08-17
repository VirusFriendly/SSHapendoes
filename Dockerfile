FROM ubuntu:20.04

RUN apt update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends tzdata && apt install openssh-server build-essential libpam0g-dev sudo rsyslog -y

RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 test 

#RUN  echo 'test:test' | chpasswd

COPY . /build
WORKDIR /build
RUN make
RUN make install
RUN cp nsswitch.conf /etc/nsswitch.conf
RUN cp sshd_config /etc/ssh/sshd_config
RUN cp sshd /etc/pam.d/sshd
RUN service rsyslog start
RUN service ssh start

EXPOSE 2222

CMD ["/build/entry.sh"]
