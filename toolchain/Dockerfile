FROM alpine

ADD build-toolchain.sh /opt/build-toolchain.sh

RUN /opt/build-toolchain.sh

ENV PATH "/opt/toolchain:$PATH"
ENV MITTOS64 "true"
ENV BUILDROOT "/opt/"
WORKDIR /opt
