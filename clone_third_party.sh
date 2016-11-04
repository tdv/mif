mkdir third_party_sources ||:
cd third_party_sources
git clone --branch v1.2.8 https://github.com/madler/zlib.git
git clone --branch boost-1.62.0 https://github.com/boostorg/boost.git
git clone --branch 1.7.7 https://github.com/open-source-parsers/jsoncpp.git
git clone --branch release-2.1.6-beta https://github.com/libevent/libevent.git
