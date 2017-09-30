// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/*
This test program is typically run indirectly, by the "puffs test" or "puffs
bench" commands. These commands take an optional "-mimic" flag to check that
Puffs' output mimics (i.e. exactly matches) other libraries' output, such as
giflib for GIF, libpng for PNG, etc.

To manually run this test:

for cc in clang gcc; do
  $cc -std=c99 -Wall -Werror flate.c && ./a.out
  rm -f a.out
done

Each edition should print "PASS", amongst other information, and exit(0).

Add the "puffs mimic cflags" (everything after the colon below) to the C
compiler flags (after the .c file) to run the mimic tests.

To manually run the benchmarks, replace "-Wall -Werror" with "-O3" and replace
the first "./a.out" with "./a.out -bench". Combine these changes with the
"puffs mimic cflags" to run the mimic benchmarks.
*/

// !! puffs mimic cflags: -DPUFFS_MIMIC -lz

#include "../../../gen/c/std/flate.c"
#include "../testlib/testlib.c"

const char* proc_filename = "std/flate.c";

// ---------------- Golden Tests

// The src_offset0 and src_offset1 magic numbers come from:
//
// go run script/extract-flate-offsets.go test/testdata/*.gz
//
// The empty comments forces clang-format to place one element per line.

golden_test checksum_midsummer_gt = {
    .src_filename = "../../testdata/midsummer.txt",  //
};

golden_test checksum_pi_gt = {
    .src_filename = "../../testdata/pi.txt",  //
};

golden_test flate_256_bytes_gt = {
    .want_filename = "../../testdata/256.bytes",    //
    .src_filename = "../../testdata/256.bytes.gz",  //
    .src_offset0 = 20,                              //
    .src_offset1 = 281,                             //
};

golden_test flate_midsummer_gt = {
    .want_filename = "../../testdata/midsummer.txt",    //
    .src_filename = "../../testdata/midsummer.txt.gz",  //
    .src_offset0 = 24,                                  //
    .src_offset1 = 5166,                                //
};

golden_test flate_pi_gt = {
    .want_filename = "../../testdata/pi.txt",    //
    .src_filename = "../../testdata/pi.txt.gz",  //
    .src_offset0 = 17,                           //
    .src_offset1 = 48335,                        //
};

golden_test flate_romeo_gt = {
    .want_filename = "../../testdata/romeo.txt",    //
    .src_filename = "../../testdata/romeo.txt.gz",  //
    .src_offset0 = 20,                              //
    .src_offset1 = 550,                             //
};

golden_test flate_romeo_fixed_gt = {
    .want_filename = "../../testdata/romeo.txt",                  //
    .src_filename = "../../testdata/romeo.txt.fixed-huff.flate",  //
};

golden_test gzip_midsummer_gt = {
    .want_filename = "../../testdata/midsummer.txt",    //
    .src_filename = "../../testdata/midsummer.txt.gz",  //
};

golden_test gzip_pi_gt = {
    .want_filename = "../../testdata/pi.txt",    //
    .src_filename = "../../testdata/pi.txt.gz",  //
};

golden_test zlib_midsummer_gt = {
    .want_filename = "../../testdata/midsummer.txt",      //
    .src_filename = "../../testdata/midsummer.txt.zlib",  //
};

golden_test zlib_pi_gt = {
    .want_filename = "../../testdata/pi.txt",      //
    .src_filename = "../../testdata/pi.txt.zlib",  //
};

// ---------------- Flate Tests

const char* puffs_flate_decode(puffs_base_buf1* dst, puffs_base_buf1* src) {
  puffs_flate_decoder dec;
  puffs_flate_decoder_constructor(&dec, PUFFS_VERSION, 0);
  puffs_base_writer1 dst_writer = {.buf = dst};
  puffs_base_reader1 src_reader = {.buf = src};
  puffs_flate_status s =
      puffs_flate_decoder_decode(&dec, dst_writer, src_reader);
  if (s) {
    return puffs_flate_status_string(s);
  }
  return NULL;
}

const char* puffs_zlib_decode(puffs_base_buf1* dst, puffs_base_buf1* src) {
  puffs_flate_zlib_decoder dec;
  puffs_flate_zlib_decoder_constructor(&dec, PUFFS_VERSION, 0);
  puffs_base_writer1 dst_writer = {.buf = dst};
  puffs_base_reader1 src_reader = {.buf = src};
  puffs_flate_status s =
      puffs_flate_zlib_decoder_decode(&dec, dst_writer, src_reader);
  if (s) {
    return puffs_flate_status_string(s);
  }
  return NULL;
}

void test_puffs_flate_decode_256_bytes() {
  proc_funcname = __func__;
  test_buf1_buf1(puffs_flate_decode, &flate_256_bytes_gt);
}

void test_puffs_flate_decode_midsummer() {
  proc_funcname = __func__;
  test_buf1_buf1(puffs_flate_decode, &flate_midsummer_gt);
}

void test_puffs_flate_decode_pi() {
  proc_funcname = __func__;
  test_buf1_buf1(puffs_flate_decode, &flate_pi_gt);
}

void test_puffs_flate_decode_romeo() {
  proc_funcname = __func__;
  test_buf1_buf1(puffs_flate_decode, &flate_romeo_gt);
}

void test_puffs_flate_decode_romeo_fixed() {
  proc_funcname = __func__;
  test_buf1_buf1(puffs_flate_decode, &flate_romeo_fixed_gt);
}

void test_puffs_flate_decode_split_src() {
  proc_funcname = __func__;

  puffs_base_buf1 src = {.ptr = global_src_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 got = {.ptr = global_got_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 want = {.ptr = global_want_buffer, .len = BUFFER_SIZE};

  golden_test* gt = &flate_256_bytes_gt;
  if (!read_file(&src, gt->src_filename)) {
    return;
  }
  if (!read_file(&want, gt->want_filename)) {
    return;
  }

  puffs_flate_decoder dec;
  puffs_base_writer1 dst_writer = {.buf = &got};
  puffs_base_reader1 src_reader = {.buf = &src};

  int i;
  for (i = 1; i < 32; i++) {
    size_t split = gt->src_offset0 + i;
    if (split >= gt->src_offset1) {
      FAIL("i=%d: split was not an interior split", i);
      return;
    }
    got.wi = 0;

    puffs_flate_decoder_constructor(&dec, PUFFS_VERSION, 0);

    src.closed = false;
    src.ri = gt->src_offset0;
    src.wi = split;
    puffs_flate_status s0 =
        puffs_flate_decoder_decode(&dec, dst_writer, src_reader);

    src.closed = true;
    src.ri = split;
    src.wi = gt->src_offset1;
    puffs_flate_status s1 =
        puffs_flate_decoder_decode(&dec, dst_writer, src_reader);

    if (s0 != PUFFS_FLATE_SUSPENSION_SHORT_READ) {
      FAIL("i=%d: s0: got %" PRIi32 " (%s), want %" PRIi32 " (%s)", i, s0,
           puffs_flate_status_string(s0), PUFFS_FLATE_SUSPENSION_SHORT_READ,
           puffs_flate_status_string(PUFFS_FLATE_SUSPENSION_SHORT_READ));
      return;
    }

    if (s1 != PUFFS_FLATE_STATUS_OK) {
      FAIL("i=%d: s1: got %" PRIi32 " (%s), want %" PRIi32 " (%s)", i, s1,
           puffs_flate_status_string(s1), PUFFS_FLATE_STATUS_OK,
           puffs_flate_status_string(PUFFS_FLATE_STATUS_OK));
      return;
    }

    char prefix[64];
    snprintf(prefix, 64, "i=%d: ", i);
    if (!buf1s_equal(prefix, &got, &want)) {
      return;
    }
  }
}

bool do_test_puffs_flate_history(int i,
                                 golden_test* gt,
                                 puffs_base_buf1* src,
                                 puffs_base_buf1* got,
                                 puffs_flate_decoder* dec,
                                 uint32_t starting_history_index,
                                 uint64_t limit,
                                 puffs_flate_status want_s) {
  src->ri = gt->src_offset0;
  src->wi = gt->src_offset1;
  got->ri = 0;
  got->wi = 0;

  puffs_flate_decoder_constructor(dec, PUFFS_VERSION, 0);
  puffs_base_writer1 dst_writer = {.buf = got};
  puffs_base_reader1 src_reader = {.buf = src};

  dec->private_impl.f_history_index = starting_history_index;

  dst_writer.limit.ptr_to_len = &limit;

  puffs_flate_status got_s =
      puffs_flate_decoder_decode(dec, dst_writer, src_reader);
  // TODO: should want_s's SHORT_WRITE be LIMITED_WRITE?
  if (got_s != want_s) {
    FAIL("i=%d: starting_history_index=0x%04" PRIX32
         ": decode status: got %" PRIi32 " (%s), want %" PRIi32 " (%s)",
         i, starting_history_index, got_s, puffs_flate_status_string(got_s),
         want_s, puffs_flate_status_string(want_s));
    return false;
  }
  return true;
}

void test_puffs_flate_history_full() {
  proc_funcname = __func__;

  puffs_base_buf1 src = {.ptr = global_src_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 got = {.ptr = global_got_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 want = {.ptr = global_want_buffer, .len = BUFFER_SIZE};

  golden_test* gt = &flate_pi_gt;
  if (!read_file(&src, gt->src_filename)) {
    return;
  }
  if (!read_file(&want, gt->want_filename)) {
    return;
  }

  const int full_history_size = 0x8000;
  int i;
  for (i = -2; i <= +2; i++) {
    puffs_flate_decoder dec;
    if (!do_test_puffs_flate_history(
            i, gt, &src, &got, &dec, 0, want.wi + i,
            i >= 0 ? PUFFS_FLATE_STATUS_OK
                   : PUFFS_FLATE_SUSPENSION_SHORT_WRITE)) {
      return;
    }

    uint32_t want_history_index = i >= 0 ? 0 : full_history_size;
    if (dec.private_impl.f_history_index != want_history_index) {
      FAIL("i=%d: history_index: got %" PRIu32 ", want %" PRIu32, i,
           dec.private_impl.f_history_index, want_history_index);
      return;
    }
    if (i >= 0) {
      continue;
    }

    puffs_base_buf1 history_got = {
        .ptr = dec.private_impl.f_history,
        .len = full_history_size,
        .wi = full_history_size,
    };
    if (want.wi < full_history_size - i) {
      FAIL("i=%d: want file is too short", i);
      return;
    }
    puffs_base_buf1 history_want = {
        .ptr = global_want_buffer + want.wi - (full_history_size - i),
        .len = full_history_size,
        .wi = full_history_size,
    };
    if (!buf1s_equal("", &history_got, &history_want)) {
      return;
    }
  }
}

void test_puffs_flate_history_partial() {
  proc_funcname = __func__;

  puffs_base_buf1 src = {.ptr = global_src_buffer, .len = BUFFER_SIZE};
  puffs_base_buf1 got = {.ptr = global_got_buffer, .len = BUFFER_SIZE};

  golden_test* gt = &flate_pi_gt;
  if (!read_file(&src, gt->src_filename)) {
    return;
  }

  uint32_t starting_history_indexes[] = {
      0x0000, 0x0001, 0x1234, 0x7FFB, 0x7FFC, 0x7FFD, 0x7FFE, 0x7FFF,
      0x8000, 0x8001, 0x9234, 0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE, 0xFFFF,
  };

  int i;
  for (i = 0; i < PUFFS_TESTLIB_ARRAY_SIZE(starting_history_indexes); i++) {
    uint32_t starting_history_index = starting_history_indexes[i];

    // The flate_pi_gt golden test file decodes to the digits of pi.
    const char* fragment = "3.14";
    const uint32_t fragment_length = 4;

    puffs_flate_decoder dec;
    if (!do_test_puffs_flate_history(i, gt, &src, &got, &dec,
                                     starting_history_index, fragment_length,
                                     PUFFS_FLATE_SUSPENSION_SHORT_WRITE)) {
      return;
    }

    bool got_full = dec.private_impl.f_history_index >= 0x8000;
    uint32_t got_history_index = dec.private_impl.f_history_index & 0x7FFF;
    bool want_full = (starting_history_index + fragment_length) >= 0x8000;
    uint32_t want_history_index =
        (starting_history_index + fragment_length) & 0x7FFF;
    if ((got_full != want_full) || (got_history_index != want_history_index)) {
      FAIL("i=%d: starting_history_index=0x%04" PRIX32
           ": history_index: got %d;%04" PRIX32 ", want %d;%04" PRIX32,
           i, starting_history_index, (int)(got_full), got_history_index,
           (int)(want_full), want_history_index);
      return;
    }

    int j;
    for (j = 0; j < fragment_length; j++) {
      uint32_t index = (starting_history_index + j) & 0x7FFF;
      uint8_t got = dec.private_impl.f_history[index];
      uint8_t want = fragment[j];
      if (got != want) {
        FAIL("i=%d: starting_history_index=0x%04" PRIX32
             ": j=%d: got 0x%02" PRIX8 ", want 0x%02" PRIX8,
             i, starting_history_index, j, got, want);
        return;
      }
    }
  }
}

void test_puffs_flate_table_redirect() {
  proc_funcname = __func__;

  // Call init_huff with a Huffman code that looks like:
  //
  //           code_bits  cl   c   r   s          1st  2nd
  //  0b_______________0   1   1   1   0  0b........0
  //  0b______________10   2   1   1   1  0b.......01
  //  0b_____________110   3   1   1   2  0b......011
  //  0b____________1110   4   1   1   3  0b.....0111
  //  0b__________1_1110   5   1   1   4  0b....01111
  //  0b_________11_1110   6   1   1   5  0b...011111
  //  0b________111_1110   7   1   1   6  0b..0111111
  //                       8   0   2
  //  0b_____1_1111_1100   9   1   3   7  0b001111111
  //  0b____11_1111_1010  10   1   5   8  0b101111111  0b..0   (3 bits)
  //                      11   0  10
  //  0b__1111_1110_1100  12  19  19   9  0b101111111  0b001
  //  0b__1111_1110_1101  12      18  10  0b101111111  0b101
  //  0b__1111_1110_1110  12      17  11  0b101111111  0b011
  //  0b__1111_1110_1111  12      16  12  0b101111111  0b111
  //  0b__1111_1111_0000  12      15  13  0b011111111  0b000   (3 bits)
  //  0b__1111_1111_0001  12      14  14  0b011111111  0b100
  //  0b__1111_1111_0010  12      13  15  0b011111111  0b010
  //  0b__1111_1111_0011  12      12  16  0b011111111  0b110
  //  0b__1111_1111_0100  12      11  17  0b011111111  0b001
  //  0b__1111_1111_0101  12      10  18  0b011111111  0b101
  //  0b__1111_1111_0110  12       9  19  0b011111111  0b011
  //  0b__1111_1111_0111  12       8  20  0b011111111  0b111
  //  0b__1111_1111_1000  12       7  21  0b111111111  0b.000  (4 bits)
  //  0b__1111_1111_1001  12       6  22  0b111111111  0b.100
  //  0b__1111_1111_1010  12       5  23  0b111111111  0b.010
  //  0b__1111_1111_1011  12       4  24  0b111111111  0b.110
  //  0b__1111_1111_1100  12       3  25  0b111111111  0b.001
  //  0b__1111_1111_1101  12       2  26  0b111111111  0b.101
  //  0b__1111_1111_1110  12       1  27  0b111111111  0b.011
  //  0b1_1111_1111_1110  13   2   1  28  0b111111111  0b0111
  //  0b1_1111_1111_1111  13       0  29  0b111111111  0b1111
  //
  // cl  is the code_length.
  // c   is counts[code_length]
  // r   is the number of codes (of that code_length) remaining.
  // s   is the symbol
  // 1st is the key in the first level table (9 bits).
  // 2nd is the key in the second level table (variable bits).

  puffs_flate_decoder dec;
  puffs_flate_decoder_constructor(&dec, PUFFS_VERSION, 0);

  // The constructor should zero out dec's fields, but to be paranoid, we zero
  // it out explicitly.
  memset(&dec.private_impl.f_huffs[0], 0, sizeof(dec.private_impl.f_huffs[0]));

  int i;
  int n = 0;
  dec.private_impl.f_code_lengths[n++] = 1;
  dec.private_impl.f_code_lengths[n++] = 2;
  dec.private_impl.f_code_lengths[n++] = 3;
  dec.private_impl.f_code_lengths[n++] = 4;
  dec.private_impl.f_code_lengths[n++] = 5;
  dec.private_impl.f_code_lengths[n++] = 6;
  dec.private_impl.f_code_lengths[n++] = 7;
  dec.private_impl.f_code_lengths[n++] = 9;
  dec.private_impl.f_code_lengths[n++] = 10;
  for (i = 0; i < 19; i++) {
    dec.private_impl.f_code_lengths[n++] = 12;
  }
  dec.private_impl.f_code_lengths[n++] = 13;
  dec.private_impl.f_code_lengths[n++] = 13;

  puffs_flate_status s = puffs_flate_decoder_init_huff(&dec, 0, 0, n, 257);
  if (s) {
    FAIL("%s", puffs_flate_status_string(s));
    return;
  }

  // There is one 1st-level table (9 bits), and three 2nd-level tables (3, 3
  // and 4 bits). f_huffs[0]'s elements should be non-zero for those tables and
  // should be zero outside of those tables.
  const int n_f_huffs = sizeof(dec.private_impl.f_huffs[0]) /
                        sizeof(dec.private_impl.f_huffs[0][0]);
  for (i = 0; i < n_f_huffs; i++) {
    bool got = dec.private_impl.f_huffs[0][i] == 0;
    bool want = i >= (1 << 9) + (1 << 3) + (1 << 3) + (1 << 4);
    if (got != want) {
      FAIL("huffs[0][%d] == 0: got %d, want %d", i, got, want);
      return;
    }
  }

  // The redirects in the 1st-level table should be at:
  //  - 0b101111111 (0x017F) to the table offset 512 (0x0200), a 3-bit table.
  //  - 0b011111111 (0x00FF) to the table offset 520 (0x0208), a 3-bit table.
  //  - 0b111111111 (0x01FF) to the table offset 528 (0x0210), a 4-bit table.
  uint32_t got;
  uint32_t want;
  got = dec.private_impl.f_huffs[0][0x017F];
  want = 0x80020039;
  if (got != want) {
    FAIL("huffs[0][0x017F]: got 0x%08" PRIX32 ", want 0x%08" PRIX32, got, want);
    return;
  }
  got = dec.private_impl.f_huffs[0][0x00FF];
  want = 0x80020839;
  if (got != want) {
    FAIL("huffs[0][0x00FF]: got 0x%08" PRIX32 ", want 0x%08" PRIX32, got, want);
    return;
  }
  got = dec.private_impl.f_huffs[0][0x01FF];
  want = 0x80021049;
  if (got != want) {
    FAIL("huffs[0][0x01FF]: got 0x%08" PRIX32 ", want 0x%08" PRIX32, got, want);
    return;
  }

  // The first 2nd-level table should look like wants.
  const uint32_t wants[8] = {
      0x40000801, 0x40000903, 0x40000801, 0x40000B03,
      0x40000801, 0x40000A03, 0x40000801, 0x40000C03,
  };
  for (i = 0; i < 8; i++) {
    got = dec.private_impl.f_huffs[0][0x0200 + i];
    want = wants[i];
    if (got != want) {
      FAIL("huffs[0][0x%04" PRIX32 "]: got 0x%08" PRIX32 ", want 0x%08" PRIX32,
           (uint32_t)(0x0200 + i), got, want);
      return;
    }
  }
}

void test_puffs_zlib_decode_midsummer() {
  proc_funcname = __func__;
  test_buf1_buf1(puffs_zlib_decode, &zlib_midsummer_gt);
}

void test_puffs_zlib_decode_pi() {
  proc_funcname = __func__;
  test_buf1_buf1(puffs_zlib_decode, &zlib_pi_gt);
}

// ---------------- Mimic Tests

#ifdef PUFFS_MIMIC

#include "../mimiclib/flate.c"

void test_mimic_flate_decode_256_bytes() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_flate_decode, &flate_256_bytes_gt);
}

void test_mimic_flate_decode_midsummer() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_flate_decode, &flate_midsummer_gt);
}

void test_mimic_flate_decode_pi() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_flate_decode, &flate_pi_gt);
}

void test_mimic_flate_decode_romeo() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_flate_decode, &flate_romeo_gt);
}

void test_mimic_flate_decode_romeo_fixed() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_flate_decode, &flate_romeo_fixed_gt);
}

void test_mimic_gzip_decode_midsummer() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_gzip_decode, &gzip_midsummer_gt);
}

void test_mimic_gzip_decode_pi() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_gzip_decode, &gzip_pi_gt);
}

void test_mimic_zlib_decode_midsummer() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_zlib_decode, &zlib_midsummer_gt);
}

void test_mimic_zlib_decode_pi() {
  proc_funcname = __func__;
  test_buf1_buf1(mimic_zlib_decode, &zlib_pi_gt);
}

#endif  // PUFFS_MIMIC

// ---------------- Flate Benches

void bench_puffs_flate_decode_1k() {
  proc_funcname = __func__;
  bench_buf1_buf1(puffs_flate_decode, tc_dst, &flate_romeo_gt, 200000);
}

void bench_puffs_flate_decode_10k() {
  proc_funcname = __func__;
  bench_buf1_buf1(puffs_flate_decode, tc_dst, &flate_midsummer_gt, 30000);
}

void bench_puffs_flate_decode_100k() {
  proc_funcname = __func__;
  bench_buf1_buf1(puffs_flate_decode, tc_dst, &flate_pi_gt, 3000);
}

// ---------------- Mimic Benches

#ifdef PUFFS_MIMIC

void bench_mimic_adler32_10k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_adler32, tc_src, &checksum_midsummer_gt, 30000);
}

void bench_mimic_adler32_100k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_adler32, tc_src, &checksum_pi_gt, 3000);
}

void bench_mimic_crc32_10k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_crc32, tc_src, &checksum_midsummer_gt, 30000);
}

void bench_mimic_crc32_100k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_crc32, tc_src, &checksum_pi_gt, 3000);
}

void bench_mimic_flate_decode_1k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_flate_decode, tc_dst, &flate_romeo_gt, 200000);
}

void bench_mimic_flate_decode_10k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_flate_decode, tc_dst, &flate_midsummer_gt, 30000);
}

void bench_mimic_flate_decode_100k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_flate_decode, tc_dst, &flate_pi_gt, 3000);
}

void bench_mimic_gzip_decode_10k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_gzip_decode, tc_dst, &gzip_midsummer_gt, 30000);
}

void bench_mimic_gzip_decode_100k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_gzip_decode, tc_dst, &gzip_pi_gt, 3000);
}

void bench_mimic_zlib_decode_10k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_zlib_decode, tc_dst, &zlib_midsummer_gt, 30000);
}

void bench_mimic_zlib_decode_100k() {
  proc_funcname = __func__;
  bench_buf1_buf1(mimic_zlib_decode, tc_dst, &zlib_pi_gt, 3000);
}

#endif  // PUFFS_MIMIC

// ---------------- Manifest

// The empty comments forces clang-format to place one element per line.
proc tests[] = {
    // Flate Tests
    test_puffs_flate_decode_256_bytes,    //
    test_puffs_flate_decode_midsummer,    //
    test_puffs_flate_decode_pi,           //
    test_puffs_flate_decode_romeo,        //
    test_puffs_flate_decode_romeo_fixed,  //
    test_puffs_flate_decode_split_src,    //
    test_puffs_flate_history_full,        //
    test_puffs_flate_history_partial,     //
    test_puffs_flate_table_redirect,      //
    test_puffs_zlib_decode_midsummer,     //
    test_puffs_zlib_decode_pi,            //

#ifdef PUFFS_MIMIC

    // Mimic Tests
    test_mimic_flate_decode_256_bytes,    //
    test_mimic_flate_decode_midsummer,    //
    test_mimic_flate_decode_pi,           //
    test_mimic_flate_decode_romeo,        //
    test_mimic_flate_decode_romeo_fixed,  //
    test_mimic_gzip_decode_midsummer,     //
    test_mimic_gzip_decode_pi,            //
    test_mimic_zlib_decode_midsummer,     //
    test_mimic_zlib_decode_pi,            //

#endif  // PUFFS_MIMIC

    NULL,
};

// The empty comments forces clang-format to place one element per line.
proc benches[] = {
    // Flate Benches
    bench_puffs_flate_decode_1k,    //
    bench_puffs_flate_decode_10k,   //
    bench_puffs_flate_decode_100k,  //

#ifdef PUFFS_MIMIC

    // Mimic Benches
    bench_mimic_adler32_10k,        //
    bench_mimic_adler32_100k,       //
    bench_mimic_crc32_10k,          //
    bench_mimic_crc32_100k,         //
    bench_mimic_flate_decode_1k,    //
    bench_mimic_flate_decode_10k,   //
    bench_mimic_flate_decode_100k,  //
    bench_mimic_gzip_decode_10k,    //
    bench_mimic_gzip_decode_100k,   //
    bench_mimic_zlib_decode_10k,    //
    bench_mimic_zlib_decode_100k,   //

#endif  // PUFFS_MIMIC

    NULL,
};
