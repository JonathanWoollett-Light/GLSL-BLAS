#![feature(test)]
extern crate test;

use rand::Rng;
use ndarray::{arr1,Array};

#[cfg(test)]
mod benches {
    use super::*;
    use test::Bencher;

    const SIZE: usize = 819200;

    #[bench]
    fn sscal(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let vals: Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();
        let x = arr1(&vals);

        bencher.iter(|| x.mapv(|v|v*2.));
    }
    #[bench]
    fn saxpy(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let x_vals:Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();
        let y_vals:Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();

        let x = arr1(&x_vals);
        let y = arr1(&y_vals);

        bencher.iter(|| x.mapv(|v|v*2.)+&y);
    }
    #[bench]
    fn sdot(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let x_vals:Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();
        let y_vals:Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();

        let x = arr1(&x_vals);
        let y = arr1(&y_vals);

        bencher.iter(|| (&x*&y).sum());
    }
    #[bench]
    fn snrm2(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let x_vals:Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();
        let x = arr1(&x_vals);

        bencher.iter(|| x.mapv(|v|v*v).sum());
    }
    #[bench]
    fn sasum(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let x_vals:Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();
        let x = arr1(&x_vals);

        bencher.iter(|| x.mapv(|v|v.abs()).sum());
    }
    #[bench]
    fn isamax(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let x_vals:Vec<f32> = (0..SIZE).map(|_|rng.gen()).collect();
        let x = arr1(&x_vals);

        bencher.iter(|| x.iter().max_by(|a,bencher|a.partial_cmp(bencher).unwrap()));
    }
    #[bench]
    fn sgemv(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let sqrt_size = (SIZE as f32).sqrt() as usize;
        let a_vals:Vec<f32> = (0..sqrt_size * sqrt_size).map(|_|rng.gen()).collect();
        let x_vals:Vec<f32> = (0..sqrt_size).map(|_|rng.gen()).collect();
        let y_vals:Vec<f32> = (0..sqrt_size).map(|_|rng.gen()).collect();

        let a = Array::from_shape_vec((sqrt_size,sqrt_size),a_vals).unwrap();
        let x = arr1(&x_vals);
        let y = arr1(&y_vals);

        let alpha = 2f32;
        let beta = 3f32;

        bencher.iter(|| a.dot(&x).mapv(|v|v*alpha)+y.mapv(|v|v*beta));
    }
    #[bench]
    fn sgemm(bencher: &mut Bencher) {
        let mut rng = rand::thread_rng();
        let sqrt_size = (SIZE as f32).sqrt() as usize;
        let a_vals:Vec<f32> = (0..sqrt_size*sqrt_size).map(|_|rng.gen()).collect();
        let b_vals:Vec<f32> = (0..sqrt_size*sqrt_size).map(|_|rng.gen()).collect();
        let c_vals:Vec<f32> = (0..sqrt_size*sqrt_size).map(|_|rng.gen()).collect();

        let a = Array::from_shape_vec((sqrt_size,sqrt_size),a_vals).unwrap();
        let b = Array::from_shape_vec((sqrt_size,sqrt_size),b_vals).unwrap();
        let c = Array::from_shape_vec((sqrt_size,sqrt_size),c_vals).unwrap();

        let alpha = 2f32;
        let beta = 3f32;

        bencher.iter(|| a.dot(&b).mapv(|v|v*alpha)+c.mapv(|v|v*beta));
    }
}