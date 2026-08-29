# HybridZip R2: Current Architecture

This companion reflects the current source-level R2 portfolio: all 43 decoder-visible HZ02 modes, the three-layer Auto router, complete archive-byte comparison, and strict per-block decode checks are integrated. The current-hash E5 router ledger and the single-worker Fast K=4 E6 performance package are complete; dashed items are limited to held-out promotion and future GPU work.

The fixed-position 16:9 figure is `hybridzip-current-architecture.svg` and its PNG export.

```mermaid
%% Solid = source-integrated or current measured capability. Dashed = held-out or future work.
flowchart LR
    input[输入文件] --> blocks[HZ02 分块<br/>默认 64 KiB]
    blocks --> layerA[Layer A<br/>结构与表示激活]
    layerA --> layerB[Layer B<br/>算法族门控]
    layerB --> layerC[Layer C<br/>候选策略]
    families[43 条已接入路径<br/>通用多编码 / LZ 表示 / specialist / neural / donor PAQ8px] --> layerC
    layerC --> compare[真实完整 HZ02<br/>archive bytes 比较]
    compare --> archive[写入 HZ02 块<br/>mode / transform / entropy / CRC32]
    archive --> decoder[严格解码<br/>边界、metadata、CRC]
    decoder --> output[逐字节一致输出]
    input --> hz01[HZ01 V1 基线<br/>四专家 mixer] --> hz01archive[HZ01 归档]
    compare --> telemetry[Auto block telemetry<br/>candidates / selected / oracle gap]
    telemetry --> ledger[当前 hash 完整账本<br/>E5 432/432 PASS；Fast E6 432/432 PASS]

    classDef current fill:#FFFFFF,stroke:#163A5F,stroke-width:2px,color:#111827;
    classDef baseline fill:#FFFFFF,stroke:#6B7280,stroke-width:1.5px,color:#374151;
    classDef pending fill:#FFFFFF,stroke:#6B7280,stroke-width:1.5px,stroke-dasharray:6 4,color:#374151;
    class input,blocks,layerA,layerB,layerC,families,compare,archive,decoder,output,telemetry current;
    class hz01,hz01archive baseline;
    class ledger current;
    linkStyle default stroke:#163A5F,stroke-width:2px;
```
