cmake_minimum_required(VERSION 2.6)

include(../build_tools/cmake/Library.cmake)
include_directories("${PROJECT_SOURCE_DIR}/src")

add_definitions(
	-DSECP256K1_WIDEMUL_INT64=1
    -DECMULT_WINDOW_SIZE=15
    -DECMULT_GEN_PREC_BITS=4
    -DSECP256K1_API=
)

add_definitions(
#    -DENABLE_MODULE_GENERATOR=1
#    -DENABLE_MODULE_RANGEPROOF=1
    -DENABLE_MODULE_ECDH=1
    -DENABLE_MODULE_RECOVERY=1
#    -DENABLE_MODULE_EXTRAKEYS=1
#    -DENABLE_MODULE_SCHNORRSIG=1
#    -DENABLE_MODULE_ECDSA_S2C=1
#    -DENABLE_MODULE_ECDSA_ADAPTOR=1
#    -DENABLE_MODULE_MUSIG=1
#    -DENABLE_MODULE_WHITELIST=1
#    -DENABLE_MODULE_SURJECTIONPROOF=1
)