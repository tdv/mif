mkdir third_party_sources ||:
cd third_party_sources
git clone --branch v1.3 https://github.com/madler/zlib.git

wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
tar zxvf boost_1_84_0.tar.gz
mv boost_1_84_0 boost
rm boost_1_84_0.tar.gz

# In Ubuntu you might have to install some additional packages
# - liblz4-dev
# - pkgconf
# - bison
# - flex

wget https://github.com/postgres/postgres/archive/REL_16_1.tar.gz
tar zxvf REL_16_1.tar.gz
mv postgres-REL_16_1 libpq
rm REL_16_1.tar.gz
