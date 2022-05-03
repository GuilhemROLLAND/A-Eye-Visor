# shipsnet

## Checklist
On picture bateau3599.jpg with rescal_fl32.json  
| Type                 | Status Python       | Status C                     | Remarque   |
| :------------------- | :------------------ | :--------------------------- | :--------- |
| Chargement image raw | image_raw           | OK                           | Same       |
| Rescale Image        | image_rescaled.json | image_preprocessed.json      | Same       |
| Weights 1            | weights10.json      | weights10.json               | Same       |
| Bias 1               | bias10.json         | weights10.json, fin (865)    | Same       |
| Layer 1 result       | layer10.json        | layer10.json                 | Leger Diff |
| Layer 2 result       | layer11.json        | layer11.json                 | Leger Diff |
| Weights 3            | weights12.json      | weights12.json               | Same       |
| Bias 3               | bias12.json         | weights12.json, fin (9217)   | Same       |
| Layer 3 result       | layer12.json        | layer12.json                 | Leger Diff |
| Weights 18           | weights18.json      | weights18.json               | Same       |
| Bias 18              | bias18.json         | weigths18.json, fin (200705) | Same       |
| Layer 18 result      | layer18.json        | layer18.json                 | Val Diff   |
| Weights 19           | weights19.json      | weights19.json               | Same       |
| Bias 19              | bias19.json         | weigths19.json, fin (65)     | Same       |
| Layer 19 result      | layer19.json        | layer19.json                 | Leger Diff |
