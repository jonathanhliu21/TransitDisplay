#ifndef DISPLAY_HELPERS_H
#define DISPLAY_HELPERS_H


/**
 * @brief Converts a 24-bit integer hex color to a 16-bit RGB565 color value.
 * @param hexcolor A uint32_t representing the color, e.g., 0xFF5733.
 * @return A uint16_t containing the color in RGB565 format.
 */
inline uint16_t hexToRGB565(uint32_t hexcolor) {
  // Extract 8-bit R, G, B components from the 24-bit integer
  uint8_t r8 = (hexcolor >> 16) & 0xFF;
  uint8_t g8 = (hexcolor >> 8) & 0xFF;
  uint8_t b8 = hexcolor & 0xFF;

  // Convert 8-bit R, G, B to 5-bit, 6-bit, and 5-bit values
  uint16_t r5 = (r8 * 249 + 1014) >> 11;
  uint16_t g6 = (g8 * 253 + 505) >> 10;
  uint16_t b5 = (b8 * 249 + 1014) >> 11;

  // Combine the components into a single 16-bit RGB565 value
  return (r5 << 11) | (g6 << 5) | b5;
}

#endif