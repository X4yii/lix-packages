extern crate alloc;
use alloc::vec::Vec;


pub fn adler32(data: &[u8]) -> u32 {
    let mut a: u32 = 1;
    let mut b: u32 = 0;
    const MOD_ADLER: u32 = 65521;

    for &byte in data {
        a = (a + byte as u32) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    (b << 16) | a
}

pub fn crc32(data: &[u8]) -> u32 {
    let mut crc: u32 = 0xFFFF_FFFF;
    for &byte in data {
        crc ^= byte as u32;
        for _ in 0..8 {
            let mask = -( (crc & 1) as i32 ) as u32;
            crc = (crc >> 1) ^ (0xEDB8_8320 & mask);
        }
    }
    !crc
}

pub fn compress_uncompressed(data: &[u8]) -> Vec<u8> {
    let mut out = Vec::with_capacity(data.len() + 6 + (data.len() / 65535 + 1) * 5);
    out.push(0x78);
    out.push(0x01);

    let mut offset = 0;
    while offset < data.len() {
        let chunk_size = core::cmp::min(data.len() - offset, 65535);
        let is_last = (offset + chunk_size) >= data.len();

        let bfinal_btype: u8 = if is_last { 0x01 } else { 0x00 };
        out.push(bfinal_btype);

        let len = chunk_size as u16;
        let nlen = !len;

        out.extend_from_slice(&len.to_le_bytes());
        out.extend_from_slice(&nlen.to_le_bytes());
        out.extend_from_slice(&data[offset..offset + chunk_size]);

        offset += chunk_size;
    }

    if data.is_empty() {
        out.push(0x01);
        out.extend_from_slice(&0u16.to_le_bytes());
        out.extend_from_slice(&(!0u16).to_le_bytes());
    }

    let checksum = adler32(data);
    out.extend_from_slice(&checksum.to_be_bytes());

    out
}

pub fn decompress_uncompressed(data: &[u8]) -> Result<Vec<u8>, &'static str> {
    if data.len() < 6 {
        return Err("Buffer demasiado corto para encabezado zlib");
    }

    let cmf = data[0];
    let flg = data[1];

    if ((cmf as u16 * 256 + flg as u16) % 31) != 0 {
        return Err("Cabecera zlib invalida");
    }

    if (flg & 0x20) != 0 {
        return Err("Diccionario predefinido no soportado");
    }

    let mut out = Vec::new();
    let mut idx = 2;

    while idx + 4 < data.len() - 4 {
        let header = data[idx];
        let is_last = (header & 0x01) != 0;
        let btype = (header >> 1) & 0x03;

        idx += 1;

        if btype != 0 {
            return Err("Solo bloques no comprimidos soportados en decoder basico");
        }

        if idx + 4 > data.len() - 4 {
            return Err("Bloque truncado");
        }

        let len = u16::from_le_bytes([data[idx], data[idx + 1]]) as usize;
        let nlen = u16::from_le_bytes([data[idx + 2], data[idx + 3]]);
        idx += 4;

        if (len as u16) != !nlen {
            return Err("Longitud de bloque inconsistente con NLEN");
        }

        if idx + len > data.len() - 4 {
            return Err("Datos de bloque incompletos");
        }

        out.extend_from_slice(&data[idx..idx + len]);
        idx += len;

        if is_last {
            break;
        }
    }

    if data.len() - idx < 4 {
        return Err("Falta suma de verificacion Adler-32");
    }

    let expected_adler = u32::from_be_bytes([data[idx], data[idx + 1], data[idx + 2], data[idx + 3]]);
    let calculated_adler = adler32(&out);

    if expected_adler != calculated_adler {
        return Err("Discrepancia en suma de verificacion Adler-32");
    }

    Ok(out)
}

#[no_mangle]
pub extern "C" fn zlibVersion() -> *const core::ffi::c_char {
    b"1.3.1\0".as_ptr() as *const core::ffi::c_char
}

#[no_mangle]
pub extern "C" fn z_adler32(adler: u32, buf: *const u8, len: usize) -> u32 {
    if buf.is_null() {
        return 1;
    }
    let slice = unsafe { core::slice::from_raw_parts(buf, len) };
    let mut a = adler & 0xffff;
    let mut b = (adler >> 16) & 0xffff;
    const MOD_ADLER: u32 = 65521;
    for &byte in slice {
        a = (a + byte as u32) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    (b << 16) | a
}

#[no_mangle]
pub extern "C" fn z_crc32(crc: u32, buf: *const u8, len: usize) -> u32 {
    if buf.is_null() {
        return 0;
    }
    let slice = unsafe { core::slice::from_raw_parts(buf, len) };
    let mut c = !crc;
    for &byte in slice {
        c ^= byte as u32;
        for _ in 0..8 {
            let mask = -((c & 1) as i32) as u32;
            c = (c >> 1) ^ (0xEDB8_8320 & mask);
        }
    }
    !c
}

#[no_mangle]
pub extern "C" fn compress(
    dest: *mut u8,
    dest_len: *mut usize,
    source: *const u8,
    source_len: usize,
) -> i32 {
    if dest.is_null() || dest_len.is_null() || (source.is_null() && source_len > 0) {
        return -2;
    }
    let src_slice = unsafe { core::slice::from_raw_parts(source, source_len) };
    let compressed = compress_uncompressed(src_slice);
    let max_len = unsafe { *dest_len };
    if compressed.len() > max_len {
        return -5;
    }
    unsafe {
        core::ptr::copy_nonoverlapping(compressed.as_ptr(), dest, compressed.len());
        *dest_len = compressed.len();
    }
    0
}

#[no_mangle]
pub extern "C" fn uncompress(
    dest: *mut u8,
    dest_len: *mut usize,
    source: *const u8,
    source_len: usize,
) -> i32 {
    if dest.is_null() || dest_len.is_null() || (source.is_null() && source_len > 0) {
        return -2;
    }
    let src_slice = unsafe { core::slice::from_raw_parts(source, source_len) };
    match decompress_uncompressed(src_slice) {
        Ok(decompressed) => {
            let max_len = unsafe { *dest_len };
            if decompressed.len() > max_len {
                return -5;
            }
            unsafe {
                core::ptr::copy_nonoverlapping(decompressed.as_ptr(), dest, decompressed.len());
                *dest_len = decompressed.len();
            }
            0
        }
        Err(_) => -3,
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_adler32() {
        assert_eq!(adler32(b"Wikipedia"), 0x11E60398);
    }

    #[test]
    fn test_crc32() {
        assert_eq!(crc32(b"123456789"), 0xCBF43926);
    }

    #[test]
    fn test_zlib_roundtrip() {
        let original = b"Hello, LixOS modern capability operating system!";
        let compressed = compress_uncompressed(original);
        assert!(!compressed.is_empty());
        let decompressed = decompress_uncompressed(&compressed).unwrap();
        assert_eq!(decompressed.as_slice(), original);
    }
}
