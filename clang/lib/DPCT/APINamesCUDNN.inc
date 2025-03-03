//===--------------------APINamesCUDNN.inc----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-----------------------------------------------------------------===//

// Base API
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        CALL_FACTORY_ENTRY("cudnnGetVersion",
                                           CALL(MapNames::getDpctNamespace() +
                                                "dnnl::get_version")))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnCreate", DEREF(ARG_WC(0)), false,
                            "create_engine")))

REMOVE_API_FACTORY_ENTRY("cudnnDestroy")

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetStream", ARG_WC(0), false, "set_queue",
                            ARG_WC(1))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudnnGetStream", DEREF(ARG_WC(1)),
                            MEMBER_CALL(ARG_WC(0), false, "get_queue"))))

// Memory API
REMOVE_API_FACTORY_ENTRY("cudnnCreateTensorDescriptor")

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetTensor4dDescriptor", ARG_WC(0), false,
                            "set", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5), ARG_WC(6))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetTensor4dDescriptorEx", ARG_WC(0), false,
                            "set", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5), ARG_WC(6), ARG_WC(7), ARG_WC(8),
                            ARG_WC(9))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetTensorNdDescriptor", ARG_WC(0), false,
                            "set", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetTensorNdDescriptorEx", ARG_WC(0), false,
                            "set", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4))))

REMOVE_API_FACTORY_ENTRY("cudnnDestroyTensorDescriptor")

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetTensor4dDescriptor", ARG_WC(0), false,
                            "get", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5), ARG_WC(6), ARG_WC(7), ARG_WC(8),
                            ARG_WC(9))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetTensorNdDescriptor", ARG_WC(0), false,
                            "get", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudnnGetTensorSizeInBytes", DEREF(ARG_WC(1)),
                            MEMBER_CALL(ARG_WC(0), false, "get_size"))))

// Simple Operation
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnTransformTensor", ARG_WC(0), false,
                            "async_reorder", DEREF(ARG_WC(1)), ARG_WC(2),
                            ARG_WC(3), DEREF(ARG_WC(4)), ARG_WC(5), ARG_WC(6))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetTensor", ARG_WC(0), false, "async_fill",
                            ARG_WC(1), ARG_WC(2), ARG_WC(3))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnScaleTensor", ARG_WC(0), false, "async_scale",
                            DEREF(ARG_WC(3)), ARG_WC(1), ARG_WC(2))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnAddTensor", ARG_WC(0), false, "async_sum",
                            DEREF(ARG_WC(1)), ARG_WC(2), ARG_WC(3),
                            DEREF(ARG_WC(4)), ARG_WC(5), ARG_WC(6))))

// Activation Layer
REMOVE_API_FACTORY_ENTRY("cudnnCreateActivationDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyActivationDescriptor")

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetActivationDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnSetActivationDescriptor", ARG_WC(0), false,
                              "set", ARG_WC(1), ARG_WC(3))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnGetActivationDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnGetActivationDescriptor", ARG_WC(0), false,
                              "get", ARG_WC(1), ARG_WC(3))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetActivationDescriptorSwishBeta", ARG_WC(0),
                            false, "set_beta", ARG_WC(1))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudnnGetActivationDescriptorSwishBeta",
                            DEREF(ARG_WC(1)),
                            MEMBER_CALL(ARG_WC(0), false, "get_beta"))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnActivationForward", ARG_WC(0), false,
                            "async_activation_forward", ARG_WC(1),
                            DEREF(ARG_WC(2)), ARG_WC(3), ARG_WC(4),
                            DEREF(ARG_WC(5)), ARG_WC(6), ARG_WC(7))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnActivationBackward", ARG_WC(0), false,
                            "async_activation_backward", ARG_WC(1),
                            DEREF(ARG_WC(2)), ARG_WC(3), ARG_WC(4), ARG_WC(5),
                            ARG_WC(6), ARG_WC(7), ARG_WC(8), DEREF(ARG_WC(9)),
                            ARG_WC(10), ARG_WC(11))))

// LRN Layer
REMOVE_API_FACTORY_ENTRY("cudnnCreateLRNDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyLRNDescriptor")

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetLRNDescriptor", ARG_WC(0), false, "set",
                            ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetLRNDescriptor", ARG_WC(0), false, "get",
                            ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnLRNCrossChannelForward", ARG_WC(0), false,
                            "async_lrn_forward", ARG_WC(1), DEREF(ARG_WC(3)),
                            ARG_WC(4), ARG_WC(5), DEREF(ARG_WC(6)), ARG_WC(7),
                            ARG_WC(8))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnLRNCrossChannelBackward",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnLRNCrossChannelBackward", ARG_WC(0), false,
                              "async_lrn_backward", ARG_WC(1), DEREF(ARG_WC(3)),
                              ARG_WC(4), ARG_WC(5), ARG_WC(6), ARG_WC(7),
                              ARG_WC(8), ARG_WC(9), DEREF(ARG_WC(10)),
                              ARG_WC(11), ARG_WC(12))),
                          Diagnostics::PRIMITIVE_WORKSPACE, ARG("async_lrn_backward"),
                          ARG("async_lrn_forward")))

// Pooling Layer
REMOVE_API_FACTORY_ENTRY("cudnnCreatePoolingDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyPoolingDescriptor")

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetPooling2dDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnSetPooling2dDescriptor", ARG_WC(0), false,
                              "set", ARG_WC(1), ARG_WC(3), ARG_WC(4), ARG_WC(5),
                              ARG_WC(6), ARG_WC(7), ARG_WC(8))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetPoolingNdDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnSetPoolingNdDescriptor", ARG_WC(0), false,
                              "set", ARG_WC(1), ARG_WC(3), ARG_WC(4), ARG_WC(5),
                              ARG_WC(6))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnGetPooling2dDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnGetPooling2dDescriptor", ARG_WC(0), false,
                              "get", ARG_WC(1), ARG_WC(3), ARG_WC(4), ARG_WC(5),
                              ARG_WC(6), ARG_WC(7), ARG_WC(8))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnGetPoolingNdDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnGetPoolingNdDescriptor", ARG_WC(0), false,
                              "get", ARG_WC(1), ARG_WC(2), ARG_WC(4), ARG_WC(5),
                              ARG_WC(6), ARG_WC(7))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetPooling2dForwardOutputDim", ARG_WC(0),
                            false, "get_forward_output_dim", ARG_WC(1),
                            ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(5))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetPoolingNdForwardOutputDim", ARG_WC(0),
                            false, "get_forward_output_dim", ARG_WC(1),
                            ARG_WC(2), ARG_WC(3))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnPoolingForward", ARG_WC(0), false,
                            "async_pooling_forward", ARG_WC(1),
                            DEREF(ARG_WC(2)), ARG_WC(3), ARG_WC(4),
                            DEREF(ARG_WC(5)), ARG_WC(6), ARG_WC(7))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnPoolingBackward",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnPoolingBackward", ARG_WC(0), false,
                              "async_pooling_backward", ARG_WC(1),
                              DEREF(ARG_WC(2)), ARG_WC(3), ARG_WC(4), ARG_WC(5),
                              ARG_WC(6), ARG_WC(7), ARG_WC(8), DEREF(ARG_WC(9)),
                              ARG_WC(10), ARG_WC(11))),
                          Diagnostics::PRIMITIVE_WORKSPACE,
                          ARG("async_pooling_backward"),
                          ARG("async_pooling_forward")))

// Softmax Layer
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSoftmaxForward", ARG_WC(0), false,
                            "async_softmax_forward", ARG_WC(1), ARG_WC(2),
                            DEREF(ARG_WC(3)), ARG_WC(4), ARG_WC(5),
                            DEREF(ARG_WC(6)), ARG_WC(7), ARG_WC(8))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnSoftmaxBackward", ARG_WC(0), false, "async_softmax_backward",
        ARG_WC(1), ARG_WC(2), DEREF(ARG_WC(3)), ARG_WC(4), ARG_WC(5), ARG_WC(6),
        ARG_WC(7), DEREF(ARG_WC(8)), ARG_WC(9), ARG_WC(10))))

// Reduce Layer
REMOVE_API_FACTORY_ENTRY("cudnnCreateReduceTensorDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyReduceTensorDescriptor")

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetReduceTensorDescriptor",
                          ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                              "cudnnSetReduceTensorDescriptor", ARG_WC(0),
                              ARG_WC(1))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("computation datatype, Nan numbers propagation "
                              "and reduction index option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnGetReduceTensorDescriptor",
                          ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                              "cudnnGetReduceTensorDescriptor",
                              DEREF(ARG_WC(1)), ARG_WC(0))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("computation datatype, Nan numbers propagation "
                              "and reduction index option")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY("cudnnGetReductionWorkspaceSize",
                                        DEREF(ARG_WC(4)), ARG("0")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnReduceTensor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnReduceTensor", ARG_WC(0), false,
                              "async_reduction", ARG_WC(1), DEREF(ARG_WC(6)),
                              ARG_WC(7), ARG_WC(8), DEREF(ARG_WC(9)),
                              ARG_WC(10), ARG_WC(11))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("reduction index")))

// OpTensor Layer
REMOVE_API_FACTORY_ENTRY("cudnnCreateOpTensorDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyOpTensorDescriptor")

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetOpTensorDescriptor",
                          ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                              "cudnnSetOpTensorDescriptor", ARG_WC(0),
                              ARG_WC(1))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("computation datatype, Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY(
        "cudnnGetOpTensorDescriptor",
        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY("cudnnGetOpTensorDescriptor",
                                                DEREF(ARG_WC(1)), ARG_WC(0))),
        Diagnostics::API_NOT_MIGRATED,
        ARG("computation datatype, Nan numbers propagation option")))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnOpTensor", ARG_WC(0), false, "async_binary",
                            ARG_WC(1), DEREF(ARG_WC(2)), ARG_WC(3), ARG_WC(4),
                            DEREF(ARG_WC(5)), ARG_WC(6), ARG_WC(7),
                            DEREF(ARG_WC(8)), ARG_WC(9), ARG_WC(10))))

// Batch Normalization Layer
FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnBatchNormalizationForwardInference", ARG_WC(0), false,
        "async_batch_normalization_forward_inference", ARG_WC(1), ARG_WC(13),
        DEREF(ARG_WC(2)), ARG_WC(4), ARG_WC(5), DEREF(ARG_WC(3)), ARG_WC(6),
        ARG_WC(7), ARG_WC(8), ARG_WC(9), ARG_WC(10), ARG_WC(11), ARG_WC(12))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnBatchNormalizationForwardTraining", ARG_WC(0), false,
        "async_batch_normalization_forward_training", ARG_WC(1), ARG_WC(14),
        ARG_WC(11), DEREF(ARG_WC(2)), ARG_WC(4), ARG_WC(5), DEREF(ARG_WC(3)),
        ARG_WC(6), ARG_WC(7), ARG_WC(8), ARG_WC(9), ARG_WC(10), ARG_WC(12),
        ARG_WC(13), ARG_WC(15), ARG_WC(16))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnBatchNormalizationForwardTrainingEx", ARG_WC(0), false,
        "async_batch_normalization_forward_training", ARG_WC(1), ARG_WC(2),
        ARG_WC(20), ARG_WC(17), ARG_WC(14), DEREF(ARG_WC(3)), ARG_WC(5),
        ARG_WC(6), DEREF(ARG_WC(4)), ARG_WC(9), ARG_WC(10), ARG_WC(7),
        ARG_WC(8), ARG_WC(11), ARG_WC(12), ARG_WC(13), ARG_WC(15), ARG_WC(16),
        ARG_WC(18), ARG_WC(19), ARG_WC(24), ARG_WC(23))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnBatchNormalizationBackward", ARG_WC(0), false,
        "async_batch_normalization_backward", ARG_WC(1), ARG_WC(16), DEREF(ARG_WC(2)),
        ARG_WC(6), ARG_WC(7), ARG_WC(8), ARG_WC(9), DEREF(ARG_WC(3)),
        ARG_WC(10), ARG_WC(11), DEREF(ARG_WC(4)), ARG_WC(12), ARG_WC(13),
        DEREF(ARG_WC(5)), ARG_WC(14), ARG_WC(15), ARG_WC(17), ARG_WC(18))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnBatchNormalizationBackwardEx", ARG_WC(0), false,
        "async_batch_normalization_backward", ARG_WC(1), ARG_WC(2), ARG_WC(25),
        ARG_WC(22), DEREF(ARG_WC(3)), ARG_WC(7), ARG_WC(8), ARG_WC(9),
        ARG_WC(10), ARG_WC(11), ARG_WC(12), DEREF(ARG_WC(4)), ARG_WC(15),
        ARG_WC(16), ARG_WC(13), ARG_WC(14), DEREF(ARG_WC(5)), ARG_WC(17),
        ARG_WC(18), ARG_WC(19), DEREF(ARG_WC(6)), ARG_WC(20), ARG_WC(21),
        ARG_WC(23), ARG_WC(24), ARG_WC(29), ARG_WC(28))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    CALL_FACTORY_ENTRY("cudnnDeriveBNTensorDescriptor",
                       CALL(MapNames::getDpctNamespace() +
                                "dnnl::derive_batch_normalization_memory_desc",
                            ARG_WC(0), ARG_WC(1), ARG_WC(2))))

ASSIGNABLE_FACTORY(
    ASSIGN_FACTORY_ENTRY("cudnnGetBatchNormalizationBackwardExWorkspaceSize",
                         DEREF(ARG_WC(10)), ARG("0")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetBatchNormalizationForwardTrainingExWorkspaceSize",
    DEREF(ARG_WC(8)), ARG("0")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cudnnGetBatchNormalizationTrainingExReserveSpaceSize",
        DEREF(ARG_WC(5)),
        MEMBER_CALL(ARG_WC(0), false, "get_batch_normalization_workspace_size",
                    ARG_WC(2), ARG_WC(4)))))

// Normalization Layer
FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    CALL_FACTORY_ENTRY("cudnnDeriveNormTensorDescriptor",
                       CALL(MapNames::getDpctNamespace() +
                                "dnnl::derive_batch_normalization_memory_desc",
                            ARG_WC(0), ARG_WC(1), ARG_WC(2), ARG_WC(3))))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetNormalizationForwardTrainingWorkspaceSize",
    DEREF(ARG_WC(10)), ARG("0")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetNormalizationBackwardWorkspaceSize",
    DEREF(ARG_WC(12)), ARG("0")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cudnnGetNormalizationTrainingReserveSpaceSize", DEREF(ARG_WC(6)),
        MEMBER_CALL(ARG_WC(0), false, "get_batch_normalization_workspace_size",
                    ARG_WC(2), ARG_WC(5)))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnNormalizationForwardInference", ARG_WC(0), false,
        "async_batch_normalization_forward_inference", ARG_WC(1), ARG_WC(2),
        ARG_WC(16), ARG_WC(19), DEREF(ARG_WC(4)), ARG_WC(6), ARG_WC(7),
        DEREF(ARG_WC(5)), ARG_WC(17), ARG_WC(18), ARG_WC(14), ARG_WC(15),
        ARG_WC(8), ARG_WC(9), ARG_WC(10), ARG_WC(11), ARG_WC(12), ARG_WC(13))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnNormalizationForwardTraining", ARG_WC(0), false,
        "async_batch_normalization_forward_training", ARG_WC(1), ARG_WC(2),
        ARG_WC(18), ARG_WC(15), ARG_WC(11), DEREF(ARG_WC(4)), ARG_WC(6),
        ARG_WC(7), DEREF(ARG_WC(5)), ARG_WC(21), ARG_WC(22), ARG_WC(19),
        ARG_WC(20), ARG_WC(8), ARG_WC(9), ARG_WC(10), ARG_WC(12), ARG_WC(13),
        ARG_WC(14), ARG_WC(16), ARG_WC(17), ARG_WC(26), ARG_WC(25))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnNormalizationBackward", ARG_WC(0), false,
        "async_batch_normalization_backward", ARG_WC(1), ARG_WC(2), ARG_WC(27),
        ARG_WC(23), DEREF(ARG_WC(4)), ARG_WC(8), ARG_WC(9), ARG_WC(10),
        ARG_WC(11), ARG_WC(12), ARG_WC(13), DEREF(ARG_WC(5)), ARG_WC(16),
        ARG_WC(17), ARG_WC(14), ARG_WC(15), DEREF(ARG_WC(6)), ARG_WC(18),
        ARG_WC(19), ARG_WC(20), DEREF(ARG_WC(7)), ARG_WC(21), ARG_WC(22),
        ARG_WC(24), ARG_WC(25), ARG_WC(26), ARG_WC(31), ARG_WC(30))))

// Convolution Layer
REMOVE_API_FACTORY_ENTRY("cudnnCreateFilterDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyFilterDescriptor")

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetFilter4dDescriptor", ARG_WC(0), false,
                            "get", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5), ARG_WC(6))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetFilterNdDescriptor", ARG_WC(0), false,
                            "get", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudnnGetFilterSizeInBytes", DEREF(ARG_WC(1)),
                            MEMBER_CALL(ARG_WC(0), false, "get_size"))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetFilter4dDescriptor", ARG_WC(0), false,
                            "set", ARG_WC(2), ARG_WC(1), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5), ARG_WC(6))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetFilterNdDescriptor", ARG_WC(0), false,
                            "set", ARG_WC(2), ARG_WC(1), ARG_WC(3), ARG_WC(4))))

REMOVE_API_FACTORY_ENTRY("cudnnCreateConvolutionDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyConvolutionDescriptor")

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudnnGetConvolutionGroupCount", DEREF(ARG_WC(1)),
                            MEMBER_CALL(ARG_WC(0), false, "get_group_count"))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetConvolutionGroupCount", ARG_WC(0), false,
                            "set_group_count", ARG_WC(1))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetConvolution2dDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnSetConvolution2dDescriptor", ARG_WC(0), false,
                              "set", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                              ARG_WC(5), ARG_WC(6))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("convolution mode and computation datatype option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetConvolutionNdDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnSetConvolutionNdDescriptor", ARG_WC(0), false,
                              "set", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("convolution mode and computation datatype option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnGetConvolution2dDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnGetConvolution2dDescriptor", ARG_WC(0), false,
                              "get", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                              ARG_WC(5), ARG_WC(6))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("convolution mode and computation datatype option")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY(
        "cudnnGetConvolutionNdDescriptor",
        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
            "cudnnGetConvolutionNdDescriptor", ARG_WC(0), false, "get",
            ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(5))),
        Diagnostics::API_NOT_MIGRATED,
        ARG("convolution mode and computation datatype option")))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetConvolution2dForwardOutputDim", ARG_WC(0),
                            false, "get_forward_output_dim", ARG_WC(1),
                            ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(5),
                            ARG_WC(6))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetConvolutionNdForwardOutputDim", ARG_WC(0),
                            false, "get_forward_output_dim", ARG_WC(1),
                            ARG_WC(2), ARG_WC(3), ARG_WC(4))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnConvolutionForward", ARG_WC(0), false,
                            "async_convolution_forward", ARG_WC(6), ARG_WC(7),
                            DEREF_CAST_IF_NEED(LITERAL("float *"), ARG(1)),
                            ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(5),
                            DEREF_CAST_IF_NEED(LITERAL("float *"), ARG(10)),
                            ARG_WC(11), ARG_WC(12))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnConvolutionBiasActivationForward", ARG_WC(0),
                            false, "async_convolution_forward", ARG_WC(6),
                            ARG_WC(7), ARG_WC(15), DEREF(ARG_WC(1)), ARG_WC(2),
                            ARG_WC(3), ARG_WC(4), ARG_WC(5), DEREF(ARG_WC(10)),
                            ARG_WC(11), ARG_WC(12), ARG_WC(13), ARG_WC(14),
                            ARG_WC(16), ARG_WC(17))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnConvolutionBackwardData", ARG_WC(0), false,
                            "async_convolution_backward_data", ARG_WC(6), ARG_WC(7),
                            DEREF(ARG_WC(1)), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5), DEREF(ARG_WC(10)), ARG_WC(11),
                            ARG_WC(12))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnConvolutionBackwardFilter", ARG_WC(0), false,
        "async_convolution_backward_weight", ARG_WC(6), ARG_WC(7), DEREF(ARG_WC(1)),
        ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(5), DEREF(ARG_WC(10)),
        ARG_WC(11), ARG_WC(12))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnConvolutionBackwardBias", ARG_WC(0), false,
                            "async_convolution_backward_bias", DEREF(ARG_WC(1)),
                            ARG_WC(2), ARG_WC(3), DEREF(ARG_WC(4)), ARG_WC(5),
                            ARG_WC(6))))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetConvolutionForwardWorkspaceSize", DEREF(ARG_WC(6)), ARG("0")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetConvolutionBackwardDataWorkspaceSize", DEREF(ARG_WC(6)), ARG("0")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetConvolutionBackwardFilterWorkspaceSize", DEREF(ARG_WC(6)), ARG("0")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetConvolutionBackwardFilterAlgorithm", DEREF(ARG_WC(7)),
    ARG("dnnl::algorithm::convolution_auto")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetConvolutionBackwardDataAlgorithm", DEREF(ARG_WC(7)),
    ARG("dnnl::algorithm::convolution_auto")))

ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
    "cudnnGetConvolutionForwardAlgorithm", DEREF(ARG_WC(7)),
    ARG("dnnl::algorithm::convolution_auto")))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetConvolutionMathType", ARG_WC(0), false,
                            "set_math_mode", ARG_WC(1))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    MULTI_STMTS_FACTORY_ENTRY(
        "cudnnFindConvolutionForwardAlgorithm", true, false, true, true,
        BO(BinaryOperatorKind::BO_Assign,
           MEMBER_EXPR(DEREF(ARG_WC(7)), false, LITERAL("algo")),
           ARG("dnnl::algorithm::convolution_auto")),
        BO(BinaryOperatorKind::BO_Assign, DEREF(ARG_WC(6)), ARG("1"))))

// RNN Layer
REMOVE_API_FACTORY_ENTRY("cudnnCreateRNNDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnCreateRNNDataDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyRNNDescriptor")

REMOVE_API_FACTORY_ENTRY("cudnnDestroyRNNDataDescriptor")

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetRNNDataDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnSetRNNDataDescriptor", ARG_WC(0), false,
                              "set", ARG_WC(2), ARG_WC(1), ARG_WC(3), ARG_WC(4),
                              ARG_WC(5))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("unpacked data layout")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnGetRNNDataDescriptor",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnGetRNNDataDescriptor", ARG_WC(0), false,
                              "get", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                              ARG_WC(5))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("unpacked data layout")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnSetRNNDescriptor_v8",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnSetRNNDescriptor_v8", ARG_WC(0), false,
                              "set", ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(6),
                              ARG_WC(9), ARG_WC(10), ARG_WC(11), ARG_WC(12))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("persist algorithm, input mode and dropout "
                              "operation between physical layers")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    WARNING_FACTORY_ENTRY("cudnnGetRNNDescriptor_v8",
                          ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                              "cudnnGetRNNDescriptor_v8", ARG_WC(0), false,
                              "get", ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(6),
                              ARG_WC(9), ARG_WC(10), ARG_WC(11), ARG_WC(12))),
                          Diagnostics::API_NOT_MIGRATED,
                          ARG("persist algorithm, input mode and dropout "
                              "operation between physical layers")))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetRNNWeightSpaceSize", ARG_WC(0), false,
                            "rnn_get_weight_space_size", ARG_WC(1), ARG_WC(2))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetRNNTempSpaceSizes", ARG_WC(0), false,
                            "rnn_get_scratchpad_workspace_size", ARG_WC(1),
                            ARG_WC(2), ARG_WC(3), ARG_WC(4), ARG_WC(5))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
        "cudnnRNNForward", ARG_WC(0), false, "async_rnn_forward", ARG_WC(1),
        ARG_WC(2), ARG_WC(4), ARG_WC(5), ARG_WC(6), ARG_WC(7), ARG_WC(8),
        ARG_WC(9), ARG_WC(10), ARG_WC(11), ARG_WC(12), ARG_WC(13), ARG_WC(14),
        ARG_WC(15), ARG_WC(16), ARG_WC(17), ARG_WC(18), ARG_WC(19))))

// Dropout Layer
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnCreateDropoutDescriptor", DEREF(ARG_WC(0)), false,
                            "init")))

REMOVE_API_FACTORY_ENTRY("cudnnDestroyDropoutDescriptor")

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnSetDropoutDescriptor", ARG_WC(0), false,
                            "set", ARG_WC(1), ARG_WC(2), ARG_WC(3), ARG_WC(4),
                            ARG_WC(5))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnGetDropoutDescriptor", ARG_WC(0), false,
                            "get", ARG_WC(2), ARG_WC(3), ARG_WC(4))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnRestoreDropoutDescriptor", ARG_WC(0), false,
                            "restore", ARG_WC(1), ARG_WC(2), ARG_WC(3),
                            ARG_WC(4), ARG_WC(5))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudnnDropoutGetStatesSize", DEREF(ARG_WC(1)),
                            MEMBER_CALL(ARG_WC(0), false, "get_dropout_state_size"))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudnnDropoutGetReserveSpaceSize", DEREF(ARG_WC(1)),
                            CALL(MapNames::getDpctNamespace() +
                            "dnnl::engine_ext::get_dropout_workspace_size",
                            ARG_WC(0)))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnDropoutForward", ARG_WC(0), false,
                            "async_dropout_forward", ARG_WC(1), ARG_WC(2),
                            ARG_WC(3), ARG_WC(4), ARG_WC(5), ARG_WC(6),
                            ARG_WC(7))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cudnnDropoutBackward", ARG_WC(0), false,
                            "async_dropout_backward", ARG_WC(1), ARG_WC(2),
                            ARG_WC(3), ARG_WC(4), ARG_WC(5), ARG_WC(6),
                            ARG_WC(7))))
