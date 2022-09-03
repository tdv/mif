mkdir third_party_sources ||:
cd third_party_sources
git clone --branch v1.2.9 https://github.com/madler/zlib.git

wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz
tar zxvf boost_1_76_0.tar.gz
mv boost_1_76_0 boost
rm boost_1_76_0.tar.gz

wget https://github.com/postgres/postgres/archive/REL9_6_2.tar.gz
tar zxvf REL9_6_2.tar.gz
mv postgres-REL9_6_2 libpq
rm REL9_6_2.tar.gz
