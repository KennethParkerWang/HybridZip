# HybridZip R2: Current Architecture

The fixed-position 16:9 PPT figure is `hybridzip-current-architecture.svg` and its PNG export. This Mermaid source remains a compact logical companion.

```mermaid
%% Logical Mermaid companion. Use hybridzip-current-architecture.svg/.png for the fixed 16:9 PPT figure.
%% Solid = implemented. Dashed = planned but not integrated.
flowchart LR
    input[输入] --> blocks[分块] --> compare[HZ02 候选比较<br/>24 条已实现路径] --> archive[写入 HZ02<br/>模式 / 变换 / CRC] --> decoder[严格解码<br/>边界 / CRC] --> output[逐字节一致]
    input --> hz01[HZ01 基线<br/>V1 四专家 mixer] --> hz01out[HZ01 归档]
    blocks -.-> families[算法族<br/>统计 / LZ / 表示 / 专家 / 神经] -.-> layerB[Layer B<br/>族级路由] -.-> layerC[Layer C<br/>分支策略] -.-> compare
    compare -.-> oracle[Oracle 观测]
    archive -.-> metadata[增强元数据]
    decoder -.-> extension[解码器扩展]

    classDef current fill:#FFFFFF,stroke:#163A5F,stroke-width:2px,color:#111827;
    classDef baseline fill:#FFFFFF,stroke:#6B7280,stroke-width:1.5px,color:#374151;
    classDef planned fill:#FFFFFF,stroke:#6B7280,stroke-width:1.5px,stroke-dasharray:6 4,color:#374151;
    class input,blocks,compare,archive,decoder,output current;
    class hz01,hz01out baseline;
    class families,layerB,layerC,oracle,metadata,extension planned;
    linkStyle default stroke:#163A5F,stroke-width:2px;
```
