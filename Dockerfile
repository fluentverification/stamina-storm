from movesrwth/storm:ci-release

RUN apt-get update && apt-get install -y build-essential libboost-all-dev git cmake clang 

ENV STORM_DIR=/opt/storm
ENV STAMINA_HOME=/opt/stamina-storm/build
ENV STAMINA_PRISM_HOME=/opt/stamina-prism/

COPY . $STAMINA_HOME/..
RUN mkdir -p $STAMINA_HOME
WORKDIR $STAMINA_HOME
RUN export PATH=$PATH:/opt/stamina-storm/build >> ~/.shinit

RUN cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DSTORM_PATH=$STORM_DIR
RUN make -j$(nproc --all)

# Also install STAMINA/PRISM and make it available
RUN mkdir -p $STAMINA_PRISM_HOME
WORKDIR $STAMINA_PRISM_HOME
RUN perl -c $(curl https://raw.githubusercontent.com/fluentverification/stamina-prism/master/install.pl)
