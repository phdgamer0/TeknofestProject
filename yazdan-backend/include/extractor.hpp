#pragma once
#include "constants.hpp"
#include "physichs.hpp"
#include <cstdint>
#include <fstream>
#include <ios>
#include <optional>

template <const char* PATH = PIPE_FILE_PATH, typename arch = Object>
class Extractor {
 private:
   std::ifstream pipe;
   uint64_t p;
   [[nodiscard]] bool next();

 public:
   explicit Extractor() {
      pipe.open(PATH, std::ios_base::in | std::ios_base::binary);
      p = 0ull;
   }
   [[nodiscard]] std::optional<arch> extract(); // creates a new object
   [[nodiscard]] bool extract(arch& ar);        // rewrites the passed object
   void reset() {
      if (pipe.is_open()) {
         pipe.close();
      }
      pipe.clear();
      pipe.open(PATH, std::ios_base::in | std::ios_base::binary);
      p = 0ull;
   }
};

template <const char* PATH, typename arch>
[[nodiscard]] bool Extractor<PATH, arch>::next() {
   if (!pipe.is_open()) {
      pipe.open(PATH, std::ios_base::in | std::ios_base::binary);
   }
   if (!pipe.is_open()) {
      return false;
   }
   if (pipe.peek() == std::ifstream::traits_type::eof()) {
      pipe.clear();
      return false;
   }
   return true;
}

template <const char* PATH, typename arch>
[[nodiscard]] std::optional<arch> Extractor<PATH, arch>::extract() {
   arch item{};
   if (extract(item)) {
      return item;
   }
   return std::nullopt;
}

template <const char* PATH, typename arch>
[[nodiscard]] bool Extractor<PATH, arch>::extract(arch& ar) {
   if (!next()) {
      return false;
   }
   pipe.read(reinterpret_cast<char*>(&ar), sizeof(arch));
   if (pipe.gcount() == static_cast<std::streamsize>(sizeof(arch))) {
      p += sizeof(arch);
      ar.pos.CylindricalToCartesian(ar.pos);
      return true;
   }
   pipe.clear();
   return false;
}