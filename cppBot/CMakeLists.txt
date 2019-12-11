# CMakeList.txt: progetto CMake per cppBot. Includere l'origine e definire
# qui la logica specifica del progetto.
#
cmake_minimum_required (VERSION 3.8)


# Aggiungere l'origine all'eseguibile di questo progetto.
add_executable (cppBot "cppBot.cpp" "cppBot.h" "User.cpp" "User.h")

target_link_libraries(cppBot TgBot ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES})

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
