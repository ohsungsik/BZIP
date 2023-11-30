BZIP은 Burrows-Wheeler block-sorting text compression algorithm과 Huffman coding을 활용한 압축 포맷인 bzip2 파일의 압축 해제 소스입니다.

이 소스는 포트폴리오 목적으로 만들어졌습니다. 
또한 압축 해제 과정을 보기 쉽게 표현하기 위해 최적화에 대한 부분은 고려되어 있지 않습니다. 때문에 실무에 바로 적용하기에는 무리가 있습니다.

### 사용 방법
BZIP.exe < BZip2 file path >

e.g) BZIP.exe "Resources\sample1.bz2"

### 개발 환경
- OS: Windows 11 22H2
- IDE: Visual Studio 2022
- CPU: Intel I7-12700H 3GHz
- RAM: 16GB

### 개발 기간
- 5일

### 제한 사항
- Randomized 미지원
- BZip version 1 미지원

### 성능 측정
| 크기    | 속도(초) |
| ------- | -------- |
| 32kb    | 0.153    |
| 72kb    | 0.358     |
| 11.72mb | 44.967   |

#### 측정 환경
- 개발환경과 동일

### 포맷 

BZip2 포맷은 Burrows-Wheeler block-sorting text compression algorithm과 Huffman coding을 활용한 압축 포맷입니다. 


```
.magic:16                       = 'BZ' signature/magic number
.version:8                      = 'h' for Bzip2 ('H'uffman coding), '0' for Bzip1 (deprecated)
.hundred_k_blocksize:8          = '1'..'9' block-size 100 kB-900 kB (uncompressed)

.compressed_magic:48            = 0x314159265359 (BCD (pi))
.crc:32                         = checksum for this block
.randomised:1                   = 0=>normal, 1=>randomised (deprecated)
.origPtr:24                     = starting pointer into BWT for after untransform
.huffman_used_map:16            = bitmap, of ranges of 16 bytes, present/not present
.huffman_used_bitmaps:0..256    = bitmap, of symbols used, present/not present (multiples of 16)
.huffman_groups:3               = 2..6 number of different Huffman tables in use
.selectors_used:15              = number of times that the Huffman tables are swapped (each 50 symbols)
*.selector_list:1..6            = zero-terminated bit runs (0..62) of MTF'ed Huffman table (*selectors_used)
.start_huffman_length:5         = 0..20 starting bit length for Huffman deltas
*.delta_bit_length:1..40        = 0=>next symbol; 1=>alter length
                                                { 1=>decrement length;  0=>increment length } (*(symbols+2)*groups)
.contents:2..∞                  = Huffman encoded data stream until end of block (max. 7372800 bit)

.eos_magic:48                   = 0x177245385090 (BCD sqrt(pi))
.crc:32                         = checksum for whole stream
.padding:0..7                   = align to whole byte
```
출처: [WIKIPEDIA - BZip2 File format](https://en.wikipedia.org/wiki/Bzip2#File_format)


압축 과정은
1. Run-length encoding
2. Burrows–Wheeler transform
3. Move-to-front
4. Run-length encoding
5. Huffman coding.
순으로 이루어지기 때문에 위 과정을 역순으로 진행하면 압축 해제가 가능합니다.


### 압축 해제 과정
위 압축 과정을 역순으로 진행하면 압축 해제가 가능함으로 압축 해제 과정은
1. 스트림 헤더 파싱                                       = Decompress::Open()
   
2. 블럭 헤더 파싱                                         = Decompress::ReadBlockHeader()
3. Huffman tables 파싱                                   = Decompress::ReadHuffmanTrees()
4. Huffman 역변환                                        = Decompress::DecodeHuffmanTransform()
5. Run-length 디코딩                                     = Decompress::DecodeHuffmanTransform()
6. Move-to-front 역변환                                  = Decompress::DecodeHuffmanTransform()
7. Burrows-Wheeler transform 역변환                      = Decompress::DecodeBWT()
8. Run-length 디코딩                                     = Decompress::ReadNextByte()
9. CRC 체크                                              = Decompress::Read()
   
10. 스트림 푸터 파싱                                      = Decompress::ReadBlockHeader()
순으로 진행됩니다.



### 참고 자료
- https://github.com/dsnet/compress/blob/master/doc/bzip2-format.pdf
- https://en.wikipedia.org/wiki/Burrows-Wheeler_transform
- https://en.wikipedia.org/wiki/Run-length_encoding
- https://en.wikipedia.org/wiki/Bzip2
- https://en.wikipedia.org/wiki/Move-to-front_transform
