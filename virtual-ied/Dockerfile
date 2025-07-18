FROM ubuntu:24.04 AS builder
ARG IEC_61850_LIB_REPO_URL=https://github.com/mz-automation/libiec61850.git
ARG IEC_61850_LIB_REPO_TAG=v1.5.1
ARG LIBIEC_HOME=/app/libiec61850
ARG WITH_GPIO
WORKDIR /app
RUN apt-get update && apt install -y build-essential && apt install -y git && apt install -y default-jre
RUN git clone ${IEC_61850_LIB_REPO_URL}
WORKDIR ${LIBIEC_HOME}
RUN git checkout ${IEC_61850_LIB_REPO_TAG}
RUN make
RUN mkdir /usr/include/iec61850
RUN mkdir /usr/lib/iec61850
RUN cp ${LIBIEC_HOME}/build/libiec61850.a /usr/lib/iec61850
RUN cp ${LIBIEC_HOME}/config/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/hal/inc/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/common/inc/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/mms/inc/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/mms/inc_private/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/mms/asn1/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/iec61850/inc/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/iec61850/inc_private/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/goose/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/sampled_values/* /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/src/logging/*.h /usr/include/iec61850
RUN cp ${LIBIEC_HOME}/hal/tls/mbedtls/* /usr/include/iec61850
WORKDIR ${LIBIEC_HOME}/tools/model_generator
COPY virtual_ied_model.icd .
RUN java -jar genmodel.jar virtual_ied_model.icd -out virtual_ied_model
WORKDIR /app
COPY app/Makefile .
RUN mkdir src
COPY app/src/virtualied.c ./src
COPY app/src/gpio_ctrl.c ./src
RUN mkdir include
COPY app/include/gpio_ctrl.h ./include
RUN cp ${LIBIEC_HOME}/tools/model_generator/virtual_ied_model.h ./include
RUN cp ${LIBIEC_HOME}/tools/model_generator/virtual_ied_model.c ./src
RUN if [ "$WITH_GPIO" = "true" ] ; then make WITH_GPIO=true ; else make WITH_GPIO=false ; fi

FROM ubuntu:24.04
ARG LIBIEC_HOME=/app/libiec61850
ENV INTERFACE=eth0
ENV APPID=4000
WORKDIR /app
COPY --from=builder /app/virtualied .
USER 0
RUN apt-get update && apt install -y iproute2 && apt install -y rsyslog && apt install -y systemctl && systemctl enable rsyslog
RUN touch /etc/rsyslog.d/virtualied.conf && echo "if \$programname == 'virtual_ied' and \$syslogseverity == '6' then /var/log/virtualied.log \n & ~" > /etc/rsyslog.d/virtualied.conf
COPY app/virtual_ied_entry.sh .
ENTRYPOINT ["/app/virtual_ied_entry.sh", "$INTERFACE", "$APPID"]
