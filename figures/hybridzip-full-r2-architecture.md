# HybridZip R2: Full Operational Architecture

This target diagram now distinguishes integrated product capability from the remaining evidence gate. The donor warehouse, 43-mode portfolio, representation/LZ/specialist/neural branches, three router layers, multi-coder comparison, HZ02 contract, and HZ01 compatibility path are source-integrated. The dashed steps are the current-hash ledger run and the resulting measured retain/retire decision.

The fixed-position 16:9 figure is `hybridzip-full-r2-architecture.svg` and its PNG export.

```mermaid
%% Solid = source-integrated HZ02 capability. Dashed = final measured decision still pending.
flowchart LR
    warehouse[KU donor warehouse<br/>source / revision / license evidence] --> portfolio[Donor-driven portfolio<br/>43 decoder-visible modes]
    input[输入文件] --> blocks[HZ02 分块<br/>默认 64 KiB]
    blocks --> layerA[Layer A<br/>结构与表示激活]
    layerA --> layerB[Layer B<br/>算法族门控]
    layerB --> layerC[Layer C<br/>branch / model / coder policy]
    portfolio --> layerC
    layerC --> compare[完整 archive bytes<br/>候选比较与 stored fallback]
    compare --> archive[HZ02 archive<br/>mode / transform / entropy / CRC32]
    archive --> decoder[严格解码与 CRC32]
    decoder --> output[逐字节一致输出]
    input --> hz01[HZ01 V1 基线<br/>NGram / PPMD / Match / LSTM / mixer] --> hz01archive[HZ01 归档]
    compare --> telemetry[Auto telemetry<br/>selected / oracle / gap]
    telemetry -.-> ledger[当前 hash 44-package ledger]
    ledger -.-> decision[基于测量的 retain / retire decision]

    classDef current fill:#FFFFFF,stroke:#163A5F,stroke-width:2px,color:#111827;
    classDef baseline fill:#FFFFFF,stroke:#6B7280,stroke-width:1.5px,color:#374151;
    classDef pending fill:#FFFFFF,stroke:#6B7280,stroke-width:1.5px,stroke-dasharray:6 4,color:#374151;
    class warehouse,portfolio,input,blocks,layerA,layerB,layerC,compare,archive,decoder,output,telemetry current;
    class hz01,hz01archive baseline;
    class ledger,decision pending;
    linkStyle default stroke:#163A5F,stroke-width:2px;
```
