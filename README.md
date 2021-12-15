# README

Branch predictor project of [CSE 240A](https://github.com/prannoy/CSE240A) at UCSD.

## Methods

Implement GShare, Tournament and a custom perceptron predictor.

## Result

Mispredicton Rate:

|            | fp_1  | fp_2 | int_1 | Int_2 | Mm_1  | Mm_2  |
| ---------- | ----- | ---- | ----- | ----- | ----- | ----- |
| GShare     | 0.84  | 1.5  | 13.9  | 0.43  | 6.52  | 10.22 |
| Tournament | 1.22  | 9.51 | 17.92 | 0.76  | 13.73 | 11.09 |
| Custom     | 0.257 | 0.53 | 16.57 | 1.1   | 12.53 | 10.11 |


